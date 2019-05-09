#define _CRT_SECURE_NO_WARNINGS // for using std::localtime()

#include "SimpleLogger.hpp"

#include <iostream>
#include <fstream>
#include <sstream>
#include <ctime>
#include <iomanip>

namespace debug_tools {

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
		std::unique_lock<std::mutex> lock(mtx_);
		auto result = dateTimeFormat_;
		return result;
	}

	void SimpleLogger::setFileName(const std::string& file_name)
	{
		std::unique_lock<std::mutex> lock(mtx_);
		filename_ = file_name;
	}
	std::string SimpleLogger::getFileName() const
	{
		std::unique_lock<std::mutex> lock(mtx_);
		auto result = filename_;
		return result;
	}

	void SimpleLogger::log(const std::string& tag, const std::string& msg)
	{
		std::unique_lock<std::mutex> lock(mtx_);
		std::cout << getCurrentDateTime() << " " << tag << " " << msg << std::endl;
	}
	void SimpleLogger::logToFile(const std::string& tag, const std::string& msg)
	{
		std::unique_lock<std::mutex> lock(mtx_);
		std::ofstream ofs;
		ofs.open(filename_.c_str(), std::ofstream::app);
		if (!ofs.is_open()) {
			return;
		}
		ofs << getCurrentDateTime() << " " << tag << " " << msg << std::endl;
		ofs.close();
	}

}