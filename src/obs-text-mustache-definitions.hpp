#pragma once

#include <obs-frontend-api.h>
#include <obs.h>
#include <QString>
#include <QWidget>
#include <QDockWidget>
#include <QLineEdit>
#include <QLabel>
#include <QSignalMapper>
#include <memory>
#include <map>
#include <set>

#include "ui_OBSTextMustacheDefinitions.h"

class OBSTextMustacheDefinitions : public QWidget {
	Q_OBJECT

	private:
		static void obsSourceSignalHandler(void *data, calldata_t *call_data, const char* callbackMethod, bool includeSourceParam);
		static void obsSourceUpdated(void *data, calldata_t *call_data);
		static void obsSourceRemoved(void *data, calldata_t *call_data);
		void FindVariables();
		void UpdateRenderedText();
		void UpdateUI();
		std::unique_ptr<Ui_OBSTextMustacheDefinitions> ui;
		std::map<QString, QLineEdit *> textLines;
		void AddOrUpdateTemplateSource(obs_source_t *source);

		QSignalMapper *lineEditSignalMapper;

	private slots:
		void VerifyKnownTemplateSources();
		void SignalSourceUpdate(obs_source *source);
		void UpdateTemplatedValue(const QString &variable);

	public:
		OBSTextMustacheDefinitions(QWidget *parent = nullptr);
		~OBSTextMustacheDefinitions();
		inline static std::set<obs_source_t *> templateSources;
};
