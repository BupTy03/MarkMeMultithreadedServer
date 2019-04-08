#include "ConsoleLogger.hpp"

#include <iostream>

void ConsoleLogger::setDateTimeFormat(const std::string& fmt)
{
	std::unique_lock<std::mutex> lock(mtx_);
	dateTimeFormat_ = fmt;
}
std::string ConsoleLogger::getDateTimeFormat() const
{
	mtx_.lock();
	auto result = dateTimeFormat_;
	mtx_.unlock();
	return result;
}

void ConsoleLogger::log(const std::string& tag, const std::string& msg)
{
	std::unique_lock<std::mutex> lock(mtx_);
	std::cout << getCurrentDateTime(dateTimeFormat_) << " " << tag << " " << msg << std::endl;
}
