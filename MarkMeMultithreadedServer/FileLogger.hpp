#ifndef FILE_LOGGER
#define FILE_LOGGER

#include "Logger.hpp"

#include <fstream>
#include <string>
#include <mutex>

class FileLogger : public Logger
{
public:
	explicit FileLogger(int log_level = 31)
		: Logger(log_level)
		, dateTimeFormat_{ "%Y-%m-%d %H:%M:%S" }
	{}

	explicit FileLogger(const std::string& file_name, int log_level = 31)
		: Logger(log_level)
		, filename_{ file_name }
		, dateTimeFormat_{ "%Y-%m-%d %H:%M:%S" }
	{}

	virtual ~FileLogger() {}

	virtual void setDateTimeFormat(const std::string&) override;
	virtual std::string getDateTimeFormat() const override;

	virtual void setFileName(const std::string&);
	virtual std::string getFileName() const;

	virtual void log(const std::string&, const std::string&) override;

private:
	std::string filename_;
	std::ofstream ofs_;
	std::string dateTimeFormat_;
	mutable std::mutex mtx_;
};

#endif // !FILE_LOGGER