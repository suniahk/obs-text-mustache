#include <map>
#include <set>
#include <QString>
#include <vector>

class VariablesAndValues {
public:
	static VariablesAndValues *getInstance();
	static void store(obs_data_t *save_data, bool saving,
					   void *ptr);
	static void load(obs_data_t *data, void *param);
	void clear();
	void putVariable(const QString &variable);
	void putValue(const QString &variable, const QString &value);
	const std::set<QString> &getVariables();
	const QString &getValue(const QString &variable);
	bool contains(const QString &variable);

private:
	static VariablesAndValues *self;

	std::map<QString, QString> variablesAndValues;
	std::set<QString> variables;

	VariablesAndValues() {}
};
