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

	~SQLConnection() { this->close(); }

	bool open();
	bool open(const std::string& file);
	void close();

	inline bool isConnected() const { return connected_; }
	inline int getLastErrorCode() const { return errorCode_; }
	inline sqlite3* getHandle() { return db_; }

private:
	sqlite3* db_{ nullptr };
	std::string filename_;
	int errorCode_{};
	bool connected_{ false };
};

struct UniqueSQLConnection 
{
	UniqueSQLConnection(SQLConnection& conn)
		: conn_{ conn } 
	{ conn_.open(); }

	~UniqueSQLConnection() { conn_.close(); }
private:
	SQLConnection& conn_;
};

#endif // !SQLCONNECTION_HPP
