#include "SimpleLogger.hpp"

#include <iostream>
#include <fstream>

#include <boost/date_time/posix_time/posix_time.hpp>

namespace debug_tools {

	SimpleLogger& SimpleLogger::Instance()
	{
		static SimpleLogger inst;
		return inst;
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
		using boost::posix_time::second_clock;
		std::unique_lock<std::mutex> lock(mtx_);
		std::cout << second_clock::local_time() << " " << tag << " " << msg << std::endl;
	}
	void SimpleLogger::logToFile(const std::string& tag, const std::string& msg)
	{
		using boost::posix_time::second_clock;
		std::unique_lock<std::mutex> lock(mtx_);
		std::ofstream ofs;
		ofs.open(filename_.c_str(), std::ofstream::app);
		if (!ofs.is_open()) {
			return;
		}
		ofs << second_clock::local_time() << " " << tag << " " << msg << std::endl;
	}

}