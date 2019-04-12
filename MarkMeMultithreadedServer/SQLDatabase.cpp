#include "SQLDatabase.hpp"

static int callback(void* query_results, int count_values, char** values, char** columns)
{
	if (query_results == nullptr) {
		return 1;
	}
	try {
		auto results = reinterpret_cast<SQLDatabase::SQLQueryResults*>(query_results);
		results->clear();
		results->reserve(count_values);
		for (int i = 0; i < count_values; ++i) {
			results->emplace_back();
			(results->back()).emplace(columns[i], ((values[i] != nullptr) ? values[i] : "NULL"));
		}
	}
	catch (...) {
		return 2;
	}
	return 0;
}

bool SQLDatabase::execute(SQLConnection& conn, const std::string& query)
{
	if (!conn.isConnected()) {
		return false;
	}

	char* err;
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