#include "SQLConnection.hpp"

bool SQLConnection::open()
{
	if (connected_) {
		return true;
	}
	if (filename_.empty()) {
		return false;
	}
	errorCode_ = sqlite3_open(filename_.c_str(), &db_);
	connected_ = (errorCode_ == 0);
	return connected_;
}
bool SQLConnection::open(const std::string& file)
{
	filename_ = file;
	errorCode_ = sqlite3_open(filename_.c_str(), &db_);
	connected_ = (errorCode_ == 0);
	return connected_;
}
void SQLConnection::close()
{
	if (db_ == nullptr) {
		return;
	}
	sqlite3_close(db_);
	db_ = nullptr;
}