#include <map>
#include <set>
#include <QString>
#include <vector>
#include <obs-frontend-api.h>
#include <obs.hpp>

class VariablesAndValues {
public:
	static VariablesAndValues *getInstance();
	void storeAll();
	std::map<QString, QString> getAll();
	void updateVariables(std::set<QString> updatedList);
	void putVariable(const QString &variable);
	void putValue(const QString &variable, const QString &value);
	const std::set<QString> &getVariables();
	const QString getValue(const QString &variable);
	bool contains(const QString &variable);

private:
	static VariablesAndValues *self;
	obs_data_t *dataStorage;
	char *dataStoragePath;
	std::set<QString> variables;

	VariablesAndValues() {}
	~VariablesAndValues();
};
