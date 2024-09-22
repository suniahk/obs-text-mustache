#pragma once

#include <obs-frontend-api.h>
#include <QString>
#include <QDockWidget>
#include <QLineEdit>
#include <obs.hpp>
#include <memory>
#include <map>

#include "ui_obs-text-mustache-definitions.h"

class OBSTextMustacheDefinitions : public QWidget {
	Q_OBJECT

public:
	OBSTextMustacheDefinitions(QWidget *parent = nullptr);
	~OBSTextMustacheDefinitions();

private slots:
	static void OBSSignal(void *data, const char *signal,
			      calldata_t *call_data);
	static void OBSEvent(enum obs_frontend_event event, void *);
	static bool UpdateUI(void *param, obs_source_t *source);

private:
	std::unique_ptr<Ui_OBSTextMustacheDefinitions> ui;
	std::map<QString, QLineEdit *> textLines;
	static bool FindVariables(void *data, obs_source_t *source);
	static bool UpdateVariables(void *param, obs_source_t *source);
	static bool UpdateText(void *data, obs_source_t *source);
	void UpdateAll();
	//void UpdateVariablesAndValues();
};