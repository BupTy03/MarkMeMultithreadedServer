#ifndef SQLDATABASE_HPP
#define SQLDATABASE_HPP

#include "sqlite3.h"
#include "SQLConnection.hpp"

#include <string>
#include <vector>
#include <map>

class SQLDatabase
{
public:
	SQLDatabase() {}

	bool execute(const SQLConnection& conn, const std::string& query);

	inline int getLastErrorCode() const { return lastErrorCode_; }
	inline std::string getLastErrorMessage() const { return lastErrorStr_; }

	using SQLQueryResults = std::vector<std::map<std::string, std::string>>;

	inline SQLQueryResults getLastQueryResults() { return queryResults_; }

private:
	SQLQueryResults queryResults_;
	std::string lastErrorStr_;
	int lastErrorCode_{};
};

#endif // !SQLDATABASE_HPP
