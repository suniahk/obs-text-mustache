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

		self->dataStorage = obs_data_create();
		self->dataStoragePath = obs_module_config_path("variables.json");

		auto *existingData = obs_data_create_from_json_file_safe(self->dataStoragePath, "bak");
		obs_data_apply(self->dataStorage, existingData);
	}
	return self;
}

void VariablesAndValues::updateVariables(std::set<QString> updatedList) {
	for(auto &variable : variables) {
		if(!updatedList.count(variable)) {
			obs_data_erase(dataStorage, variable.toUtf8());
		}
	}
	
	variables.clear();
	variables = updatedList;
}

void VariablesAndValues::putVariable(const QString &variable)
{
	blog(LOG_DEBUG, "VariablesAndValues::putVariable: Parsing found variable %s",
	     variable.toStdString());

	if(!variables.count(variable)) {
		variables.insert(variable);
	}
}

bool VariablesAndValues::contains(const QString &variable)
{
	return variables.count(variable) != 0;
}

void VariablesAndValues::putValue(const QString &variable, const QString &value)
{
	blog(LOG_INFO, "VariablesAndValues::putValue: Variable %s, Value %s",
	     variable.toStdString(), value.toStdString());

	putVariable(variable);
	obs_data_set_string(
 					dataStorage, variable.toUtf8(),
 					value.toUtf8());
}

const set<QString> &VariablesAndValues::getVariables()
{
	return variables;
}

const QString VariablesAndValues::getValue(const QString &variable)
{
	return QString::fromUtf8(obs_data_get_string(dataStorage, variable.toUtf8()));
}

void VariablesAndValues::storeAll() {
	obs_data_save_json_safe(dataStorage, dataStoragePath, "json", "bak");
}

std::map<QString, QString> VariablesAndValues::getAll() {
	std::map<QString, QString> storedValues;
	for(const auto &variable : variables) {
		storedValues[variable] = getValue(variable);
	}
	return storedValues;
}

VariablesAndValues::~VariablesAndValues() {
	bfree(dataStoragePath);
	obs_data_release(dataStorage);
}
