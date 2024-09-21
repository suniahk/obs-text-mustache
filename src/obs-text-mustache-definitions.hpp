#pragma once

#include <QTimer>
#include <QString>
#include <QLineEdit>
#include <memory>
#include <map>

#include "ui_obs-text-mustache-definitions.h"

class QCloseEvent;

class OBSTextMustacheDefinitions : public QDialog {
	Q_OBJECT

public:
	std::unique_ptr<Ui_OBSTextMustacheDefinitions> ui;
	OBSTextMustacheDefinitions(QWidget *parent);
	~OBSTextMustacheDefinitions();

	void closeEvent(QCloseEvent *event) override;

public slots:
	void UpdateVariablesAndValues();
	void UpdateUI();
	void UpdateVariables();
	void ShowDialog();
	void HideDialog();
	void OBSSignal(void *data, const char *signal,
			      calldata_t *call_data);
	void OBSEvent(enum obs_frontend_event event, void *);

private:
	std::map<QString, QLineEdit *> textLines;
};