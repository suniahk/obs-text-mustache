#include <obs-module.h>
#include <util/platform.h>
#include <util/util.hpp>
#include "variables.hpp"

using namespace std;

VariablesAndValues *VariablesAndValues::self = nullptr;

VariablesAndValues *VariablesAndValues::getInstance()
{
	if (self == nullptr) {
		self = new VariablesAndValues();
	}
	return self;
}

void VariablesAndValues::putVariable(const QString &variable)
{
	blog(LOG_INFO, "VariablesAndValues::putVariable %s",
	     variable.toStdString().c_str());
	if (variablesAndValues.find(variable) == variablesAndValues.end()) {
		variablesAndValues[variable] = QString("");
		variables.insert(variable);
		blog(LOG_INFO, "VariablesAndValues::putVariable %s init finished",
	     variable.toStdString().c_str());
	}
}

bool VariablesAndValues::contains(const QString &variable)
{
	return variables.find(variable) != variables.end();
}

void VariablesAndValues::putValue(const QString &variable, const QString &value)
{
	blog(LOG_INFO, "VariablesAndValues::putValue %s %s",
	     variable.toStdString().c_str(), value.toStdString().c_str());
	variablesAndValues[variable] = value;
	variables.insert(variable);
}

const set<QString> &VariablesAndValues::getVariables()
{
	return variables;
}

const QString &VariablesAndValues::getValue(const QString &variable)
{
	return variablesAndValues[variable];
}

void VariablesAndValues::clear()
{
	blog(LOG_INFO, "VariablesAndValues::clear");

	variablesAndValues.clear();
	variables.clear();
}

void VariablesAndValues::loadVariables(obs_data_t *data, void *param)
{
	VariablesAndValues *variablesAndValues = static_cast<VariablesAndValues *>(param);

	blog(LOG_DEBUG,
		     "VariablesAndValues::load: loading variables");
	const auto variables = variablesAndValues->getVariables();

	variablesAndValues->putValue(obs_data_get_string(data, "variable"),
				     obs_data_get_string(data, "value"));
}

void VariablesAndValues::storeVariables(obs_data_t *save_data, bool saving,
					   void *ptr)
{
	VariablesAndValues *variablesAndValues = static_cast<VariablesAndValues *>(ptr);

	if (saving) {
		const OBSDataAutoRelease obj = obs_data_create();
		const OBSDataArrayAutoRelease array = obs_data_array_create();
		const auto variables = variablesAndValues->getVariables();
		for (auto it = variables.begin(); it != variables.end(); ++it) {
			const QString variable = *it;
			const QString value = variablesAndValues->getValue(*it);
			blog(LOG_DEBUG,
			     "VariablesAndValues::store: Considering saving variable %s",
			     variable.toStdString().c_str());
			if (value.size() > 0) {
				const OBSDataAutoRelease keyValue =
					obs_data_create();
				blog(LOG_DEBUG,
				     "VariablesAndValues::store: Saving variable %s as %s",
				     variable.toStdString().c_str(),
				     value.toStdString().c_str());

				obs_data_set_string(
					keyValue, "variable",
					variable.toStdString().c_str());
				obs_data_set_string(
					keyValue, "value",
					value.toStdString().c_str());

				obs_data_array_push_back(array, keyValue);

				blog(LOG_DEBUG,
				     "VariablesAndValues::store: Done saving variable %s as %s",
				     variable.toStdString().c_str(),
				     value.toStdString().c_str());
			}
		}

		obs_data_set_array(obj, "variablesAndValues", array);
		blog(LOG_DEBUG,
		     "VariablesAndValues::store: About to save data");
		obs_data_set_obj(save_data, "obs-text-mustache", obj);
		// variablesAndValues->clear();
		blog(LOG_DEBUG,
		     "VariablesAndValues::store: Done saving data");
	} else {
		OBSDataAutoRelease obj =
			obs_data_get_obj(save_data, "obs-text-mustache");

		if (!obj)
			obj = obs_data_create();
		
		variablesAndValues->clear();
		obs_data_array_enum(obs_data_get_array(obj,
						       "variablesAndValues"),
				    loadVariables, variablesAndValues);
	}
}