#ifndef CONSOLE_LOGGER
#define CONSOLE_LOGGER

#include "Logger.hpp"

#include <mutex>

class ConsoleLogger : public Logger
{
public:
	explicit ConsoleLogger(int log_level = 31)
		: Logger(log_level)
		, dateTimeFormat_{ "%Y-%m-%d %H:%M:%S" }
	{}

	virtual ~ConsoleLogger() {}

	virtual void setDateTimeFormat(const std::string& fmt) override;
	virtual std::string getDateTimeFormat() const override;

	virtual void log(const std::string&, const std::string&) override;

private:
	std::string dateTimeFormat_;
	mutable std::mutex mtx_;
};

#endif // !CONSOLE_LOGGER