#include "FileLogger.hpp"

void FileLogger::setDateTimeFormat(const std::string& fmt)
{
	std::unique_lock<std::mutex> lock(mtx_);
	dateTimeFormat_ = fmt;
}
std::string FileLogger::getDateTimeFormat() const
{
	mtx_.lock();
	auto result = dateTimeFormat_;
	mtx_.unlock();
	return result;
}

void FileLogger::setFileName(const std::string& file_name)
{
	std::unique_lock<std::mutex> lock(mtx_);
	filename_ = file_name;
}
std::string FileLogger::getFileName() const
{
	mtx_.lock();
	auto result = filename_;
	mtx_.unlock();
	return result;
}

void FileLogger::log(const std::string& tag, const std::string& msg)
{
	std::unique_lock<std::mutex> lock(mtx_);
	ofs_.open(filename_.c_str(), std::ofstream::out | std::ofstream::app);
	if (!ofs_.is_open()) {
		return;
	}
	ofs_ << getCurrentDateTime(dateTimeFormat_) << " " << tag << " " << msg << std::endl;
	ofs_.close();
}