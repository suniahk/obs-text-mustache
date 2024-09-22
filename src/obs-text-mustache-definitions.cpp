#include <obs-module.h>
#include <set>
#include <string>
#include <regex>
#include <iterator>
#include <obs.hpp>
#include <util/util.hpp>
#include <util/platform.h>
#include <QAction>
#include <QWidget>
#include <QTimer>
#include <QObject>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>
#include "obs-text-mustache-definitions.hpp"
#include "obs-text.hpp"
#include "variables.hpp"

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

bool OBSTextMustacheDefinitions::UpdateText(void *data, obs_source_t *source)
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

	//setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

	obs_frontend_add_event_callback(OBSEvent, nullptr);

	signal_handler_connect_global(obs_get_signal_handler(), OBSSignal,
				      this);

	hide();
}

// void OBSTextMustacheDefinitions::closeEvent(QCloseEvent *)
// {
// 	//obs_frontend_save();
// }

// void OBSTextMustacheDefinitions::UpdateVariablesAndValues() {
// 	obs_enum_sources(findVariables, this);
// }

bool OBSTextMustacheDefinitions::UpdateUI(void *param, obs_source_t *source) {
	OBSTextMustacheDefinitions *mustache = static_cast<OBSTextMustacheDefinitions *>(param);
	VariablesAndValues *const variablesAndValues =
		VariablesAndValues::getInstance();
	mustache->ui->gridLayout->setColumnStretch(0, 1);
	mustache->ui->gridLayout->setColumnStretch(1, 2);
	const auto variables = variablesAndValues->getVariables();
	int currentRow = 0;
	mustache->textLines.clear();
	for (auto it = variables.begin(); it != variables.end();
	     ++it, ++currentRow) {

		QLabel *label = new QLabel(*it);
		label->setAlignment(Qt::AlignVCenter);
		mustache->ui->gridLayout->addWidget(label, currentRow, 0);
		QLineEdit *lineEdit =
			new QLineEdit(variablesAndValues->getValue(*it));
		mustache->textLines[*it] = lineEdit;
		mustache->ui->gridLayout->addWidget(lineEdit, currentRow, 1);
	}

	return true;
}

bool OBSTextMustacheDefinitions::UpdateVariables(void *param, obs_source_t *source) {
	VariablesAndValues *const variablesAndValues =
		VariablesAndValues::getInstance();
	const auto variables = variablesAndValues->getVariables();
	for (auto it = variables.begin(); it != variables.end(); ++it) {
		const auto variable = *it;
		const auto value = textLines[*it]->text();
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
	obs_enum_sources(UpdateVariables, this);
	obs_enum_sources(UpdateUI, this);
	obs_enum_sources(UpdateText, this);
}

// void OBSTextMustacheDefinitions::ShowDialog()
// {
// 	UpdateUI();
// 	setVisible(true);
// 	//QTimer::singleShot(250, this, &OBSTextMustacheDefinitions::show);
// }

// void OBSTextMustacheDefinitions::HideDialog()
// {
// 	setVisible(false);
// 	UpdateVariables();
	

// 	QTimer::singleShot(250, this, &OBSTextMustacheDefinitions::hide);
// }

// void OBSTextMustacheDefinitions::TimerTextUpdate()
// {
// 	//obs_enum_sources(updateText, NULL);
// 	//timer.start(250);
// }

void OBSTextMustacheDefinitions::OBSSignal(void *data, const char *signal,
			      calldata_t *call_data)
{
	UNUSED_PARAMETER(signal);
	obs_source_t *source =
		static_cast<obs_source_t *>(calldata_ptr(call_data, "source"));
	if (!source)
		return;

	OBSTextMustacheDefinitions *obsTextMustache = static_cast<OBSTextMustacheDefinitions *>(data);
	QMetaObject::invokeMethod(obsTextMustache, "UpdateAll",
				  Qt::QueuedConnection);
}

// static void SaveOBSTextMustacheDefinitions(obs_data_t *save_data, bool saving,
// 					   void *)
// {
// 	VariablesAndValues *variablesAndValues =
// 		VariablesAndValues::getInstance();
// 	if (saving) {
// 		const OBSDataAutoRelease obj = obs_data_create();
// 		const OBSDataArrayAutoRelease array = obs_data_array_create();
// 		const auto variables = variablesAndValues->getVariables();
// 		for (auto it = variables.begin(); it != variables.end(); ++it) {
// 			const QString variable = *it;
// 			const QString value = variablesAndValues->getValue(*it);
// 			blog(LOG_INFO,
// 			     "SaveOBSTextMustacheDefinitions: Considering saving variable %s",
// 			     variable.toStdString().c_str());
// 			if (value.size() > 0) {
// 				const OBSDataAutoRelease keyValue =
// 					obs_data_create();
// 				blog(LOG_INFO,
// 				     "SaveOBSTextMustacheDefinitions: Saving variable %s as %s",
// 				     variable.toStdString().c_str(),
// 				     value.toStdString().c_str());
// 				obs_data_set_string(
// 					keyValue, "variable",
// 					variable.toStdString().c_str());
// 				obs_data_set_string(
// 					keyValue, "value",
// 					value.toStdString().c_str());

// 				obs_data_array_push_back(array, keyValue);

// 				blog(LOG_INFO,
// 				     "SaveOBSTextMustacheDefinitions: Done saving variable %s as %s",
// 				     variable.toStdString().c_str(),
// 				     value.toStdString().c_str());
// 			}
// 		}
// 		obs_data_set_array(obj, "variablesAndValues", array);
// 		blog(LOG_INFO,
// 		     "SaveOBSTextMustacheDefinitions: About to save data");
// 		obs_data_set_obj(save_data, "obs-text-mustache", obj);
// 		// variablesAndValues->clear();
// 		blog(LOG_INFO,
// 		     "SaveOBSTextMustacheDefinitions: Done saving data");
// 	} else {
// 		OBSDataAutoRelease obj =
// 			obs_data_get_obj(save_data, "obs-text-mustache");

// 		if (!obj)
// 			obj = obs_data_create();
// 		blog(LOG_INFO,
// 		     "SaveOBSTextMustacheDefinitions: loading variables");
// 		variablesAndValues->clear();
// 		obs_data_array_enum(obs_data_get_array(obj,
// 						       "variablesAndValues"),
// 				    loadVariablesAndValues, NULL);
// 	}
// }

void OBSTextMustacheDefinitions::OBSEvent(enum obs_frontend_event event, void *ptr)
{
	OBSTextMustacheDefinitions *mustache = reinterpret_cast<OBSTextMustacheDefinitions *>(ptr);

	blog(LOG_DEBUG, "OBSEvent: %d", event);

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