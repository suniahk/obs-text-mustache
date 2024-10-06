#include "obs-text-mustache-definitions.hpp"

#include <set>
#include <map>
#include <string>
#include <regex>
#include <iterator>

#include <obs.h>
#include <obs-module.h>
#include <QMainWindow>
#include <util/platform.h>
#include <QWidget>
#include <QObject>
#include <QString>
#include <QLabel>
#include <QSignalMapper>
#include <QLineEdit>
#include "obs-text.hpp"
#include "variables.hpp"

#include "ui_OBSTextMustacheDefinitions.h"

using namespace std;

const wregex variable_regex(L"\\{\\{(\\w+)\\}\\}");

// Constructor
OBSTextMustacheDefinitions::OBSTextMustacheDefinitions(QWidget *parent)
	: QWidget(parent),
	  ui(new Ui_OBSTextMustacheDefinitions)
{
	ui->setupUi(this);

	VariablesAndValues *const variablesAndValues =
		VariablesAndValues::getInstance();

	//obs_frontend_add_event_callback(OBSEvent, this);

	auto * signalHandler = obs_get_signal_handler();
	//signal_handler_connect_global(obs_get_signal_handler(), OBSSignal, this);
	signal_handler_connect(signalHandler, "source_update", obsSourceUpdated, this);
	signal_handler_connect(signalHandler, "source_load", obsSourceUpdated, this);
	signal_handler_connect(signalHandler, "source_remove", obsSourceRemoved, this);

	hide();
}

// OBS Signal Handlers
void OBSTextMustacheDefinitions::obsSourceSignalHandler(void *data, calldata_t *call_data, const char* callbackMethod, bool includeSourceParam = false) {
	obs_source_t *source =
		static_cast<obs_source_t *>(calldata_ptr(call_data, "source"));
	const char *id = obs_source_get_id(source);

	if (!source || strcmp("text_gdiplus_mustache_v2", id))
		return;

	OBSTextMustacheDefinitions *mustache = static_cast<OBSTextMustacheDefinitions *>(data);

	if(includeSourceParam) {
		QMetaObject::invokeMethod(mustache, callbackMethod,
				  Qt::QueuedConnection, source);
	} else {
		QMetaObject::invokeMethod(mustache, callbackMethod,
				  Qt::QueuedConnection);
	}
}

void OBSTextMustacheDefinitions::obsSourceUpdated(void *data, calldata_t *call_data) {
	OBSTextMustacheDefinitions::obsSourceSignalHandler(data, call_data, "SignalSourceUpdate", true);
}

void OBSTextMustacheDefinitions::obsSourceRemoved(void *data, calldata_t *call_data) {
	OBSTextMustacheDefinitions::obsSourceSignalHandler(data, call_data, "VerifyKnownTemplateSources");
}

void OBSTextMustacheDefinitions::SignalSourceUpdate(obs_source *source) {
	blog(LOG_INFO, "OBSTextMustacheDefinitions::SignalSourceUpdate called");
	AddOrUpdateTemplateSource(source);
	UpdateUI();
	
	UpdateRenderedText();
}

// Template Searching
void OBSTextMustacheDefinitions::AddOrUpdateTemplateSource(obs_source_t *source) {
	const char *id = obs_source_get_id(source);
	auto *source_ref = obs_source_get_ref(source);

	if (!strcmp("text_gdiplus_mustache_v2", id) && !OBSTextMustacheDefinitions::templateSources.count(source_ref)) {
		OBSTextMustacheDefinitions::templateSources.insert(source_ref);
	} else {
		obs_source_release(source_ref);
	}
}

void OBSTextMustacheDefinitions::VerifyKnownTemplateSources() {
	for (auto &source_ref : OBSTextMustacheDefinitions::templateSources) {
		if(obs_source_removed(source_ref)) {
			obs_source_release(source_ref);
			OBSTextMustacheDefinitions::templateSources.erase(source_ref);
		}
	}
}

// Variable Handlers
void OBSTextMustacheDefinitions::FindVariables()
{
	VariablesAndValues *variablesAndValues =
		VariablesAndValues::getInstance();

		std::set<QString> newVariablesList;

	for (auto &source_ref : OBSTextMustacheDefinitions::templateSources) {
		TextSource *mySource = reinterpret_cast<TextSource *>(
			obs_obj_get_data(source_ref));

		const auto variables_begin =
			wsregex_iterator(mySource->text.begin(),
					 mySource->text.end(), variable_regex);
		const auto variables_end = wsregex_iterator();
		for (wsregex_iterator i = variables_begin; i != variables_end;
		     ++i) {
			const wsmatch match = *i;
			const wstring match_str = match.str(1);
			const QString variable =
				QString::fromStdWString(match_str);
					blog(LOG_INFO,
			     "findVariables: found new variable %s in the scene",
			     variable.toStdString());
					 newVariablesList.insert(variable);
		}
	}

	variablesAndValues->updateVariables(newVariablesList);
}

void OBSTextMustacheDefinitions::UpdateTemplatedValue(const QString &variable) {
	blog(LOG_DEBUG, "OBSTextMustacheDefinitions::UpdateVariables called");
	VariablesAndValues *const variablesAndValues =
		VariablesAndValues::getInstance();

		const auto value = textLines[variable]->text();
		variablesAndValues->putValue(variable, value);
		blog(LOG_DEBUG, "UpdateVariables: Setting variable %s to %s",
		     variable.toStdString(),
		     value.toStdString());

	UpdateRenderedText();
}

// Source Interaction
void OBSTextMustacheDefinitions::UpdateRenderedText()
{
	for (const auto &source_ref : OBSTextMustacheDefinitions::templateSources) {
		TextSource *mySource = reinterpret_cast<TextSource *>(
			obs_obj_get_data(source_ref));
		mySource->UpdateTextToRender();
	}
}

// Widget Rendering
void OBSTextMustacheDefinitions::UpdateUI()
{
	VariablesAndValues *const variablesAndValues =
		VariablesAndValues::getInstance();

	FindVariables();

  // Remove text fields for variables that no longer exist
	for (const auto &[textVar, textField] : textLines) {
		if(!variablesAndValues->contains(textVar)) {
			ui->gridLayout->removeRow(textField);
			textLines.erase(textVar);
		}
	}

	// Add text fields for new variables
	for(const auto &variable : variablesAndValues->getVariables()) {
		if(textLines.count(variable)) {
			continue;
		}

		QLabel *label = new QLabel(variable);
		label->setAlignment(Qt::AlignVCenter);
		QLineEdit *lineEdit =
			new QLineEdit(variablesAndValues->getValue(variable));
		QObject::connect(lineEdit, &QLineEdit::textChanged,[=](){this->UpdateTemplatedValue(variable);});
		textLines[variable] = lineEdit;
		ui->gridLayout->addRow(label, lineEdit);
	}
}

// Destructor
OBSTextMustacheDefinitions::~OBSTextMustacheDefinitions() {
	auto * signalHandler = obs_get_signal_handler();
	signal_handler_disconnect(signalHandler, "source_update", obsSourceUpdated, this);
	signal_handler_disconnect(signalHandler, "source_load", obsSourceUpdated, this);
	signal_handler_disconnect(signalHandler, "source_remove", obsSourceRemoved, this);
}
