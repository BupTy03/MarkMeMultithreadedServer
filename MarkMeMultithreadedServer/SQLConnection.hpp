#ifndef SQLCONNECTION_HPP
#define SQLCONNECTION_HPP

#include "sqlite3.h"

#include <string>

class SQLConnection
{
public:
	SQLConnection(){}
	SQLConnection(const std::string& file)
		: filename_{ file }
	{}

	~SQLConnection() { if (db_ != nullptr) sqlite3_close(db_); }

	bool open();
	bool open(const std::string& file);
	void close();

	inline bool isConnected() const { return connected_; }
	inline int getLastErrorCode() const { return errorCode_; }
	inline sqlite3* getHandle() const { return db_; }

private:
	sqlite3* db_{ nullptr };
	std::string filename_;
	int errorCode_{};
	bool connected_{ false };
};

#endif // !SQLCONNECTION_HPP
