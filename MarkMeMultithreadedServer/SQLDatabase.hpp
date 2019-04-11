#ifndef SQLDATABASE_HPP
#define SQLDATABASE_HPP

#include "sqlite3.h"
#include "SQLConnection.hpp"

#include <string>
#include <vector>
#include <map>
#include <utility>

#include <boost/format.hpp>

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

	inline static std::string formatQueryString(const std::string& query_str){ return query_str; }
	template<class Arg, class... Args>
	static std::string formatQueryString(const std::string& query_str, Arg&& arg, Args&&... args);

private:
	SQLQueryResults queryResults_;
	std::string lastErrorStr_;
	int lastErrorCode_{};
};

template<class... Args>
bool SQLDatabase::execute(const SQLConnection& conn, const std::string& query, Args&&... args)
{
	return execute(conn, formatQueryString(query, std::forward<Args>(args)...));
}

template<class Arg, class... Args>
std::string SQLDatabase::formatQueryString(const std::string& query_str, Arg&& arg, Args&&... args)
{
	boost::format formatter{ query_str };
	auto list = { &(formatter % std::forward<Arg>(arg)), &(formatter % std::forward<Args>(args))... };
	(void)list;
	return formatter.str();
}

#endif // !SQLDATABASE_HPP
