#include <obs-frontend-api.h>
#include <obs-module.h>
#include <set>
#include <string>
#include <regex>
#include <iterator>
#include <obs.hpp>
#include <util/util.hpp>
#include <QAction>
#include <QMainWindow>
#include <QDialog>
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

static bool findVariables(void *data, obs_source_t *source)
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

static bool updateText(void *data, obs_source_t *source)
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
	: QDialog(parent),
	  ui(new Ui_OBSTextMustacheDefinitions)
{
	ui->setupUi(this);

	setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

	QObject::connect(ui->buttonBox->button(QDialogButtonBox::Close),
			 &QPushButton::clicked, this,
			 &OBSTextMustacheDefinitions::hide);
	QObject::connect(ui->buttonBox->button(QDialogButtonBox::Close),
			 &QPushButton::clicked, this,
			 &OBSTextMustacheDefinitions::HideDialog);
}

void OBSTextMustacheDefinitions::closeEvent(QCloseEvent *)
{
	//obs_frontend_save();
}

void OBSTextMustacheDefinitions::UpdateVariablesAndValues() {
	obs_enum_sources(findVariables, NULL);
}

void OBSTextMustacheDefinitions::UpdateUI() {
VariablesAndValues *const variablesAndValues =
		VariablesAndValues::getInstance();
	ui->gridLayout->setColumnStretch(0, 1);
	ui->gridLayout->setColumnStretch(1, 2);
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
}

void OBSTextMustacheDefinitions::UpdateVariables() {
	VariablesAndValues *const variablesAndValues =
		VariablesAndValues::getInstance();
	const auto variables = variablesAndValues->getVariables();
	for (auto it = variables.begin(); it != variables.end(); ++it) {
		const auto variable = *it;
		const auto value = textLines[*it]->text();
		variablesAndValues->putValue(variable, value);
		blog(LOG_DEBUG, "HideDialog: Setting variable %s to %s",
		     variable.toStdString().c_str(),
		     value.toStdString().c_str());
	}
}

void OBSTextMustacheDefinitions::ShowDialog()
{
	UpdateUI();
	setVisible(true);
	//QTimer::singleShot(250, this, &OBSTextMustacheDefinitions::show);
}

void OBSTextMustacheDefinitions::HideDialog()
{
	setVisible(false);
	UpdateVariables();
	
	obs_enum_sources(updateText, NULL);

	QTimer::singleShot(250, this, &OBSTextMustacheDefinitions::hide);
}

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
	QMetaObject::invokeMethod(obsTextMustache, "UpdateVariablesAndValues",
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

void OBSTextMustacheDefinitions::OBSEvent(enum obs_frontend_event event, void *)
{
	blog(LOG_DEBUG, "OBSEvent: %d", event);

	switch (event) {
	case OBS_FRONTEND_EVENT_SCENE_CHANGED:
	case OBS_FRONTEND_EVENT_PREVIEW_SCENE_CHANGED:
		obs_enum_sources(updateText, NULL);
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

extern "C" void InitOBSTextMustacheDefinitions()
{
	QAction *const action = (QAction *)obs_frontend_add_tools_menu_qaction(
		obs_module_text("TextGDIPlusDefinitions"));

	const QMainWindow *window = (QMainWindow *)obs_frontend_get_main_window();

	obs_frontend_push_ui_translation(obs_module_get_string);

	auto *obsTextMustache = new OBSTextMustacheDefinitions(window);

	const auto cb = []() {
		obsTextMustache->ShowDialog();
	};

	obs_frontend_pop_ui_translation();

	obs_frontend_add_event_callback(obsTextMustache->OBSEvent, nullptr);

	signal_handler_connect_global(obs_get_signal_handler(), obsTextMustache->OBSSignalHandler,
				      this);

	action->connect(action, &QAction::triggered, cb);
}