#include "obs-text-mustache-definitions.hpp"

#include <set>
#include <map>
#include <string>
#include <regex>
#include <iterator>

#include <obs-module.h>
#include <QMainWindow>
#include <util/platform.h>
#include <QWidget>
#include <QObject>
#include <QString>
#include <QLabel>
#include <QLineEdit>
#include "obs-text.hpp"
#include "variables.hpp"

#include "ui_OBSTextMustacheDefinitions.h"

using namespace std;

const wregex variable_regex(L"\\{\\{(\\w+)\\}\\}");

bool OBSTextMustacheDefinitions::FindVariables(void *data, obs_source_t *source)
{
	VariablesAndValues *variablesAndValues =
		VariablesAndValues::getInstance();

	const char *id = obs_source_get_id(source);

	if (!strcmp("text_gdiplus_mustache_v2", id)) {
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
	}

	return true;
}

bool OBSTextMustacheDefinitions::UpdateRenderedText(void *data, obs_source_t *source)
{
	const char *id = obs_source_get_id(source);
	if (!strcmp("text_gdiplus_mustache_v2", id)) {
		TextSource *mySource = reinterpret_cast<TextSource *>(
			obs_obj_get_data(source));
		mySource->UpdateTextToRender();
	}

	return true;
}

// static void loadVariablesAndValues(obs_data_t *data, void *param)
// {
// 	VariablesAndValues *const variablesAndValues =
// 		VariablesAndValues::getInstance();
// 	const auto variables = variablesAndValues->getVariables();

// 	variablesAndValues->putValue(obs_data_get_string(data, "variable"),
// 				     obs_data_get_string(data, "value"));
// }

OBSTextMustacheDefinitions::OBSTextMustacheDefinitions(QWidget *parent)
	: QWidget(parent),
	  ui(new Ui_OBSTextMustacheDefinitions)
{
	ui->setupUi(this);

	VariablesAndValues *const variablesAndValues =
		VariablesAndValues::getInstance();
		

	obs_frontend_add_save_callback(VariablesAndValues::storeVariables, variablesAndValues);
	obs_frontend_add_event_callback(OBSEvent, this);

	signal_handler_connect_global(obs_get_signal_handler(), OBSSignal,
				      this);

	hide();
}

bool OBSTextMustacheDefinitions::UpdateVariables(void *data, obs_source_t *source) {
	VariablesAndValues *const variablesAndValues =
		VariablesAndValues::getInstance();

	OBSTextMustacheDefinitions *mustache = static_cast<OBSTextMustacheDefinitions *>(data);
	const auto variables = variablesAndValues->getVariables();
	for (auto it = variables.begin(); it != variables.end(); ++it) {
		const auto variable = *it;
		const auto value = mustache->textLines[*it]->text();
		variablesAndValues->putValue(variable, value);
		blog(LOG_DEBUG, "UpdateVariables: Setting variable %s to %s",
		     variable.toStdString().c_str(),
		     value.toStdString().c_str());
	}

	return true;
}

void OBSTextMustacheDefinitions::UpdateAll()
{
	obs_enum_sources(FindVariables, this);

	VariablesAndValues *const variablesAndValues =
		VariablesAndValues::getInstance();

		blog(LOG_INFO, "OBSTextMustacheDefinitions::UpdateAll Triggered");

	ui->gridLayout->setColumnStretch(0, 1);
	ui->gridLayout->setColumnStretch(1, 2);

	blog(LOG_INFO, "OBSTextMustacheDefinitions::UpdateAll GetVariables");

	const auto variables = variablesAndValues->getVariables();
	int currentRow = 0;
	textLines.clear();

	for (auto it = variables.begin(); it != variables.end();
	     ++it, ++currentRow) {
		QLabel *label = new QLabel(*it);
		label->setAlignment(Qt::AlignVCenter);
		ui->gridLayout->addWidget(label, currentRow, 0);
		QLineEdit *lineEdit =
			new QLineEdit(variablesAndValues->getValue(*it));
		textLines[*it] = lineEdit;
		ui->gridLayout->addWidget(lineEdit, currentRow, 1);
	}

	obs_enum_sources(UpdateVariables, this);
	obs_enum_sources(UpdateRenderedText, this);
}

void OBSTextMustacheDefinitions::SignalSourceUpdate() {
	blog(LOG_INFO, "OBSTextMustacheDefinitions::SignalSourceUpdate called");
	UpdateAll();
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
		mustache->UpdateAll();
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