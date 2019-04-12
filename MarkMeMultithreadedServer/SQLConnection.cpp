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
	if (!connected_ && db_ != nullptr) {
		sqlite3_close(db_);
	}
	return connected_;
}
bool SQLConnection::open(const std::string& file)
{
	if (connected_ && filename_ == file) {
		return true;
	}

	if (file.empty()) {
		close();
		return false;
	}
	filename_ = file;
	errorCode_ = sqlite3_open(filename_.c_str(), &db_);
	connected_ = (errorCode_ == 0);
	if (!connected_ && db_ != nullptr) {
		sqlite3_close(db_);
	}
	return connected_;
}
void SQLConnection::close()
{
	if (!connected_) return;
	if (db_ != nullptr) {
		sqlite3_close(db_);
		db_ = nullptr;
	}
	connected_ = false;
}