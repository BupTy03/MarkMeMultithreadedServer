// for std::localtime()
#define _CRT_SECURE_NO_WARNINGS

#include "SimpleLogger.hpp"

#include <iostream>
#include <fstream>
#include <sstream>
#include <ctime>
#include <iomanip>

SimpleLogger& SimpleLogger::Instance()
{
	static SimpleLogger inst;
	return inst;
}

std::string SimpleLogger::getCurrentDateTime() const
{
	auto t = std::time(nullptr);
	std::ostringstream os;
	os << std::put_time(std::localtime(&t), dateTimeFormat_.c_str());
	return os.str();
}

void SimpleLogger::setDateTimeFormat(const std::string& fmt)
{
	std::unique_lock<std::mutex> lock(mtx_);
	dateTimeFormat_ = fmt;
}
std::string SimpleLogger::getDateTimeFormat() const
{
	mtx_.lock();
	auto result = dateTimeFormat_;
	mtx_.unlock();
	return result;
}

void SimpleLogger::setFileName(const std::string& file_name)
{
	std::unique_lock<std::mutex> lock(mtx_);
	filename_ = file_name;
}
std::string SimpleLogger::getFileName() const
{
	mtx_.lock();
	auto result = filename_;
	mtx_.unlock();
	return result;
}

void SimpleLogger::log(const std::string& tag, const std::string& msg)
{
	std::unique_lock<std::mutex> lock(mtx_);
	if (type_ == LoggerType::FILE) {
		std::ofstream ofs;
		ofs.open(filename_.c_str(), std::ofstream::app);
		if (!ofs.is_open()) {
			return;
		}
		ofs << getCurrentDateTime() << " " << tag << " " << msg << std::endl;
		ofs.close();
	}
	else {
		std::cout << getCurrentDateTime() << " " << tag << " " << msg << std::endl;
	}
}

void SimpleLogger::info(const std::string& msg)
{
	if (!hasInfoLvl()) return;
	log("INFO", msg);
}
void SimpleLogger::debug(const std::string& msg)
{
	if (!hasDebugLvl()) return;
	log("DEBUG", msg);
}
void SimpleLogger::warning(const std::string& msg)
{
	if (!hasWarningLvl()) return;
	log("WARNING", msg);
}
void SimpleLogger::critical(const std::string& msg)
{
	if (!hasCriticalLvl()) return;
	log("CRITICAL", msg);
}
void SimpleLogger::fatal(const std::string& msg)
{
	if (!hasFatalLvl()) return;
	log("FATAL", msg);
}