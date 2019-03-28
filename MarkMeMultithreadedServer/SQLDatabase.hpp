#ifndef SQLDATABASE_HPP
#define SQLDATABASE_HPP

#include "sqlite3.h"
#include "SQLConnection.hpp"
#include "my_utils.hpp"

#include <string>
#include <vector>
#include <map>
#include <utility>

class SQLDatabase
{
public:
	SQLDatabase() {}

	bool execute(const SQLConnection& conn, const std::string& query);
	template<class... Args>
	bool execute(const SQLConnection& conn, const std::string& query, Args&&... args);

	inline int getLastErrorCode() const { return lastErrorCode_; }
	inline std::string getLastErrorMessage() const { return lastErrorStr_; }

	using SQLQueryResults = std::vector<std::map<std::string, std::string>>;

	inline SQLQueryResults getLastQueryResults() { return queryResults_; }

private:
	SQLQueryResults queryResults_;
	std::string lastErrorStr_;
	int lastErrorCode_{};
};

template<class... Args>
bool SQLDatabase::execute(const SQLConnection& conn, const std::string& query, Args&&... args)
{
	return execute(conn, my_utils::format_str(query, std::forward<Args>(args)...));
}

#endif // !SQLDATABASE_HPP
