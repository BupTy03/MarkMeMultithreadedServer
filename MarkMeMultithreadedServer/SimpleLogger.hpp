#ifndef SIMPLE_LOGGER_HPP
#define SIMPLE_LOGGER_HPP

#include <string>
#include <mutex>

#ifdef DEBUG
#ifdef LOGGING_TO_FILE
#define LogMsg(tag, message) ((debug_tools::SimpleLogger::Instance()).logToFile((tag), (message)))
#else
#define LogMsg(tag, message) ((debug_tools::SimpleLogger::Instance()).log((tag), (message)))
#endif // LOGGING_TO_FILE

#define LogDebug(message)		(LogMsg("DEBUG", (message)))
#define LogInfo(message)		(LogMsg("INFO", (message)))
#define LogWarning(message)		(LogMsg("WARNING", (message)))
#define LogCritical(message)	(LogMsg("CRITICAL", (message)))
#define LogFatal(message)		(LogMsg("FATAL", (message)))

#else
#define LogMsg(tag, message)	((void)0)

#define LogDebug(message)		((void)0)
#define LogInfo(message)		((void)0)
#define LogWarning(message)		((void)0)
#define LogCritical(message)	((void)0)
#define LogFatal(message)		((void)0)

#endif // DEBUG

namespace debug_tools {

	class SimpleLogger
	{
	public:

		static SimpleLogger& Instance();

		void setDateTimeFormat(const std::string&);
		std::string getDateTimeFormat() const;

		void setFileName(const std::string&);
		std::string getFileName() const;

		void log(const std::string&, const std::string&);
		void logToFile(const std::string&, const std::string&);

	private:
		SimpleLogger()
			: dateTimeFormat_{ "%Y-%m-%d %H:%M:%S" }
			, filename_{ "logfile.txt" }
		{}

		std::string getCurrentDateTime() const;

	private:
		std::string filename_;
		std::string dateTimeFormat_;
		mutable std::mutex mtx_;
	};

}

#endif // !SIMPLE_LOGGER_HPP
