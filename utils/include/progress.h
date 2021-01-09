#pragma once

#include <stdarg.h>
#include <string>
#include <time.h>

namespace a7az0th {

struct ProgressCallback {
public:
	enum class LogLevel {
		lowest,
		error = lowest,
		warning,
		info,
		debug,
		highest = debug
	};

	// Create a default progress. Log level is set to Info
	ProgressCallback();
	// Create a progress and set logging level to @logLevel
	ProgressCallback(LogLevel logLevel);
	~ProgressCallback();

	// Write a message in the specified layer
	void debug(const char *format, ...) const;
	void info(const char *format, ...) const;
	void error(const char *format, ...) const;
	void warning(const char *format, ...) const;

	// Set logging level to @loggingLevel
	void setLogLevel(LogLevel loggingLevel);
private:
	static std::string getTimestamp();
	//Parameters:
	LogLevel logLevel;
};

}