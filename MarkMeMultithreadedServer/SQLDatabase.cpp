#include "SQLDatabase.hpp"

int callback(void* query_results, int count_values, char** values, char** columns)
{
	if (query_results == nullptr) {
		return 1;
	}
	auto results = reinterpret_cast<SQLDatabase::SQLQueryResults*>(query_results);
	results->emplace_back();
	auto& curr_record = results->back();
	for (int i = 0; i < count_values; ++i) {
		curr_record[columns[i]] = ((values[i] != nullptr) ? values[i] : "NULL");
	}
	return 0;
}

bool SQLDatabase::execute(const SQLConnection& conn, const std::string& query)
{
	if (!conn.isConnected()) {
		return false;
	}

	char* err{ nullptr };
	queryResults_.clear();
	lastErrorCode_ = sqlite3_exec(conn.getHandle(), query.c_str(), callback, &queryResults_, &err);

	if (lastErrorCode_ != SQLITE_OK) {
		if (err != nullptr) {
			lastErrorStr_ = err;
			sqlite3_free(err);
		}
		return false;
	}

	return true;
}