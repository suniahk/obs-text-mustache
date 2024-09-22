#pragma once

#include <QString>
#include <QLineEdit>
#include <memory>
#include <map>

#include "ui_obs-text-mustache-definitions.h"

class QCloseEvent;

class OBSTextMustacheDefinitions : public QWidget {
	Q_OBJECT

public:
	OBSTextMustacheDefinitions(QWidget *parent = nullptr);
	~OBSTextMustacheDefinitions();

private slots:
	void UpdateUI();
	static void OBSSignal(void *data, const char *signal,
			      calldata_t *call_data);
	static void OBSEvent(enum obs_frontend_event event, void *);

private:
	std::unique_ptr<Ui_OBSTextMustacheDefinitions> ui;
	std::map<QString, QLineEdit *> textLines;
	void UpdateVariables();
	void ShowDialog();
	void HideDialog();
	void UpdateVariablesAndValues();
};