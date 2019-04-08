#include "Logger.hpp"

void Logger::info(const std::string& msg)
{
	if (!hasInfoLvl()) {
		return;
	}
	log("INFO", msg);
}

void Logger::debug(const std::string& msg)
{
	if (!hasDebugLvl()) {
		return;
	}
	log("DEBUG", msg);
}

void Logger::warning(const std::string& msg)
{
	if (!hasWarningLvl()) {
		return;
	}
	log("WARNING", msg);
}

void Logger::critical(const std::string& msg)
{
	if (!hasCriticalLvl()) {
		return;
	}
	log("CRITICAL", msg);
}

void Logger::fatal(const std::string& msg)
{
	if (!hasFatalLvl()) {
		return;
	}
	log("FATAL", msg);
}
