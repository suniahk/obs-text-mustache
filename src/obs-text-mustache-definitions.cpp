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

void OBSTextMustacheDefinitions::UpdateTemplateSources() {
	for (auto weak_source = templateSources.begin(); weak_source != templateSources.end(); ++weak_source) {
		obs_source_t *source = obs_weak_source_get_source(*weak_source);
		if(obs_source_removed(source)) {
			obs_weak_source_release(*weak_source);
			templateSources.erase(*weak_source);
		}

		obs_source_release(source);
	}

	obs_enum_sources(FindTemplateSources, this);
}

bool OBSTextMustacheDefinitions::FindTemplateSources(void *data, obs_source_t *source) {
	const char *id = obs_source_get_id(source);
	auto *weak_source = obs_source_get_weak_source(source);

	OBSTextMustacheDefinitions *mustache = static_cast<OBSTextMustacheDefinitions *>(data);

	if (!strcmp("text_gdiplus_mustache_v2", id) && !mustache->templateSources.count(weak_source)) {
		mustache->templateSources.insert(weak_source);
	} else {
		obs_weak_source_release(weak_source);
	}

	return true;
}

void OBSTextMustacheDefinitions::FindVariables()
{
	VariablesAndValues *variablesAndValues =
		VariablesAndValues::getInstance();

	for (auto weak_source = templateSources.begin(); weak_source != templateSources.end(); ++weak_source) {
		obs_source_t *source = obs_weak_source_get_source(*weak_source);
		TextSource *mySource = reinterpret_cast<TextSource *>(
			obs_obj_get_data(source));

		blog(LOG_DEBUG,
		     "findVariables: found text_gdiplus_mustache_v2 source with text %s",
		     QString::fromStdWString(mySource->text)
			     .toStdString()
			     .c_str());

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
			blog(LOG_DEBUG,
			     "findVariables: found variable %s in the scene",
			     variable.toStdString().c_str());
			if (!variablesAndValues->contains(variable)) {
				blog(LOG_DEBUG,
				     "findVariables: adding variable %s",
				     variable.toStdString().c_str());
				variablesAndValues->putVariable(variable);
			}
		}
		obs_source_release(source);
	}
}

void OBSTextMustacheDefinitions::UpdateRenderedText()
{
	for (const auto &weak_source : OBSTextMustacheDefinitions::templateSources) {
		obs_source_t *source = obs_weak_source_get_source(weak_source);
		TextSource *mySource = reinterpret_cast<TextSource *>(
			obs_obj_get_data(source));
		mySource->UpdateTextToRender();
		obs_source_release(source);
	}
}

OBSTextMustacheDefinitions::OBSTextMustacheDefinitions(QWidget *parent)
	: QWidget(parent),
	  ui(new Ui_OBSTextMustacheDefinitions)
{
	ui->setupUi(this);

	VariablesAndValues *const variablesAndValues =
		VariablesAndValues::getInstance();

	lineEditSignalMapper = new QSignalMapper(this);
	QObject::connect(lineEditSignalMapper,&QSignalMapper::mappedString,this,&OBSTextMustacheDefinitions::UpdateVariables);
		

	obs_frontend_add_save_callback(VariablesAndValues::storeVariables, variablesAndValues);
	obs_frontend_add_event_callback(OBSEvent, this);

	signal_handler_connect_global(obs_get_signal_handler(), OBSSignal,
				      this);

	hide();
}

void OBSTextMustacheDefinitions::UpdateVariables(const QString &variable) {
	blog(LOG_INFO, "OBSTextMustacheDefinitions::UpdateVariables called");
	VariablesAndValues *const variablesAndValues =
		VariablesAndValues::getInstance();

	//const auto variables = variablesAndValues->getVariables();
	//for(const auto &variable : variables) {
	//for (auto it = variables.begin(); it != variables.end(); ++it) {
		//const auto variable = *it;
		const auto value = textLines[variable]->text();
		variablesAndValues->putValue(variable, value);
		blog(LOG_DEBUG, "UpdateVariables: Setting variable %s to %s",
		     variable.toStdString().c_str(),
		     value.toStdString().c_str());
	//}

	UpdateRenderedText();
}

void OBSTextMustacheDefinitions::UpdateUI()
{
	FindVariables();

	VariablesAndValues *const variablesAndValues =
		VariablesAndValues::getInstance();

		blog(LOG_INFO, "OBSTextMustacheDefinitions::UpdateUI Triggered");

	//ui->gridLayout->setColumnStretch(0, 1);
	//ui->gridLayout->setColumnStretch(1, 2);

	blog(LOG_INFO, "OBSTextMustacheDefinitions::UpdateAll GetVariables");

	const auto variables = variablesAndValues->getVariables();
	//int currentRow = 0;
	//textLines.clear();

	for (const auto &[textVar, textField] : textLines) {
		if(!variables.count(textVar)) {
			ui->gridLayout->removeRow(textField);
			textField->disconnect();
			textField->deleteLater();
		}
	}

	for(const auto &variable : variables) {
	//for (auto it = variables.begin(); it != variables.end();
	//     ++it, ++currentRow) {
		if(textLines.count(variable)) {
			continue;
		}
		//auto rowCount = ui->gridLayout->rowCount();
		QLabel *label = new QLabel(variable);
		label->setAlignment(Qt::AlignVCenter);
		QLineEdit *lineEdit =
			new QLineEdit(variablesAndValues->getValue(variable));
		QObject::connect(lineEdit, &QLineEdit::textChanged, lineEditSignalMapper, qOverload<>(&QSignalMapper::map));
		textLines[variable] = lineEdit;
		ui->gridLayout->addRow(label, lineEdit);
		//ui->gridLayout->addWidget(label, rowCount, 0);
		//ui->gridLayout->addWidget(lineEdit, rowCount, 1);
	}
}

void OBSTextMustacheDefinitions::SignalSourceUpdate() {
	blog(LOG_INFO, "OBSTextMustacheDefinitions::SignalSourceUpdate called");
	UpdateTemplateSources();
	UpdateUI();
}

void OBSTextMustacheDefinitions::OBSSignal(void *data, const char *signal,
			      calldata_t *call_data)
{
	UNUSED_PARAMETER(signal);
	obs_source_t *source =
		static_cast<obs_source_t *>(calldata_ptr(call_data, "source"));
	if (!source)
		return;

	const char *id = obs_source_get_id(source);

	if(obs_source_removed(source) || strcmp("text_gdiplus_mustache_v2", id)) {
		return;
	}

	OBSTextMustacheDefinitions *mustache = static_cast<OBSTextMustacheDefinitions *>(data);
	QMetaObject::invokeMethod(mustache, "SignalSourceUpdate",
				  Qt::QueuedConnection);
}

void OBSTextMustacheDefinitions::OBSEvent(enum obs_frontend_event event, void *ptr)
{
	OBSTextMustacheDefinitions *mustache = reinterpret_cast<OBSTextMustacheDefinitions *>(ptr);

	blog(LOG_DEBUG, "OBSEvent: %d", event);

	blog(LOG_INFO, "OBSTextMustacheDefinitions::OBSEvent called");

	switch (event) {
	case OBS_FRONTEND_EVENT_SCENE_CHANGED:
	case OBS_FRONTEND_EVENT_PREVIEW_SCENE_CHANGED:
		mustache->UpdateTemplateSources();
		mustache->UpdateUI();
		break;
	default:
		break;
	}
	// switch (event) {
	// case OBS_FRONTEND_EVENT_EXIT: {
	// 	obs_frontend_save();
	// 	FreeOBSTextMustacheDefinitions();
	// 	break;
	// }
	// case OBS_FRONTEND_EVENT_SCENE_COLLECTION_CLEANUP: {
	// 	VariablesAndValues *variablesAndValues =
	// 		VariablesAndValues::getInstance();
	// 	variablesAndValues->clear();
	// 	ResetDialog();
	// }
	// case OBS_FRONTEND_EVENT_FINISHED_LOADING: {
	// 	obs_enum_sources(updateText, NULL);
	// 	break;
	// }
	// }
}

OBSTextMustacheDefinitions::~OBSTextMustacheDefinitions() {
	signal_handler_disconnect_global(obs_get_signal_handler(), OBSSignal,
					 this);
}