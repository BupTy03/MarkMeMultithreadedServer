#ifndef LOGGER
#define LOGGER

// it's necessary for using std::localtime()
#define _CRT_SECURE_NO_WARNINGS

#include <string>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <atomic>

class Logger
{
public:
	enum LogLevel
	{
		INFO = 0b10000,
		DEBUG = 0b01000,
		WARNING = 0b00100,
		CRITICAL = 0b00010,
		FATAL = 0b00001,
		NONE = 0b00000
	};

public:
	explicit Logger(int log_level)
		: logLevel_{ log_level }
	{}

	virtual ~Logger() {}

	virtual void setDateTimeFormat(const std::string& fmt) = 0;
	virtual std::string getDateTimeFormat() const = 0;

	inline void setLogLevel(int lvl) noexcept { logLevel_ = lvl; }
	inline int getLogLevel() const noexcept { return logLevel_; }

	virtual void log(const std::string&, const std::string&) = 0;

	virtual void info(const std::string&);
	virtual void debug(const std::string&);
	virtual void warning(const std::string&);
	virtual void critical(const std::string&);
	virtual void fatal(const std::string&);

protected:
	inline bool hasInfoLvl()		const noexcept { return getBit(logLevel_, 4); }
	inline bool hasDebugLvl()		const noexcept { return getBit(logLevel_, 3); }
	inline bool hasWarningLvl()		const noexcept { return getBit(logLevel_, 2); }
	inline bool hasCriticalLvl()	const noexcept { return getBit(logLevel_, 1); }
	inline bool hasFatalLvl()		const noexcept { return getBit(logLevel_, 0); }

	std::string getCurrentDateTime(const std::string& format)
	{
		auto t = std::time(nullptr);
		std::ostringstream os;
		os << std::put_time(std::localtime(&t), format.c_str());
		return os.str();
	}

private:
	inline bool getBit(int num, int k) const noexcept { return (num & (1 << k)) != 0; }
	std::atomic<int> logLevel_;
};

#endif // !LOGGER