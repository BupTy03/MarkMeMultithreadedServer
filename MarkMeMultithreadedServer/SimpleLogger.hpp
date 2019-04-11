#ifndef SIMPLE_LOGGER_HPP
#define SIMPLE_LOGGER_HPP

#include <string>
#include <mutex>
#include <atomic>

enum class LoggerType {
	CONSOLE,
	FILE
};

class SimpleLogger
{
public:
	enum LogLevel {
		INFO = 0b10000,
		DEBUG = 0b01000,
		WARNING = 0b00100,
		CRITICAL = 0b00010,
		FATAL = 0b00001,
		NONE = 0b00000
	};

	static SimpleLogger& Instance();

	void setLoggerType(LoggerType t) { type_ = t; }
	LoggerType getLoggerType() { return type_; }

	void setDateTimeFormat(const std::string&);
	std::string getDateTimeFormat() const;

	void setLogLevel(int lvl) { logLevel_ = lvl; }
	inline int getLogLevel() const { return logLevel_; }

	void setFileName(const std::string&);
	std::string getFileName() const;

	void log(const std::string&, const std::string&);

	void info(const std::string&);
	void debug(const std::string&);
	void warning(const std::string&);
	void critical(const std::string&);
	void fatal(const std::string&);

private:
	SimpleLogger()
		: logLevel_{ 0b11111 }
		, type_{ LoggerType::CONSOLE }
		, dateTimeFormat_{ "%Y-%m-%d %H:%M:%S" }
		, filename_{ "logfile.txt" }
	{}

	inline bool getBit(int num, int k) const noexcept { return (num & (1 << k)) != 0; }

	inline bool hasInfoLvl()		const noexcept { return getBit(logLevel_, 4); }
	inline bool hasDebugLvl()		const noexcept { return getBit(logLevel_, 3); }
	inline bool hasWarningLvl()		const noexcept { return getBit(logLevel_, 2); }
	inline bool hasCriticalLvl()	const noexcept { return getBit(logLevel_, 1); }
	inline bool hasFatalLvl()		const noexcept { return getBit(logLevel_, 0); }

	std::string getCurrentDateTime() const;

private:
	std::atomic<int> logLevel_;
	std::atomic<LoggerType> type_;
	std::string dateTimeFormat_;
	std::string filename_;
	mutable std::mutex mtx_;
};

#endif // !SIMPLE_LOGGER_HPP
