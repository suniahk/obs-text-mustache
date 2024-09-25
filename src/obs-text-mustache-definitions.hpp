#pragma once

#include <obs-frontend-api.h>
#include <obs.h>
#include <QString>
#include <QWidget>
#include <QDockWidget>
#include <QLineEdit>
#include <QLabel>
#include <obs.hpp>
#include <memory>
#include <map>
#include <set>

#include "ui_OBSTextMustacheDefinitions.h"

class OBSTextMustacheDefinitions : public QWidget {
	Q_OBJECT

	private:
		static void OBSSignal(void *data, const char *signal,
					calldata_t *call_data);
		static void OBSEvent(enum obs_frontend_event event, void *);
		void FindVariables();
		static bool FindTemplateSources(void *data, obs_source_t *source);
		void UpdateRenderedText();
		void UpdateUI();
		void UpdateTemplateSources();
		std::unique_ptr<Ui_OBSTextMustacheDefinitions> ui;
		std::map<QString, QLineEdit *> textLines;
		std::map<QString, QLabel *> textLabels;

		std::set<obs_weak_source_t *> templateSources;

	private slots:
		void SignalSourceUpdate();
		void UpdateVariables(const QString &text);

	public:
		OBSTextMustacheDefinitions(QWidget *parent = nullptr);
		~OBSTextMustacheDefinitions();
};