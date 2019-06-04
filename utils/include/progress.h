#pragma once

#include <stdarg.h>
#include <string>
#include <time.h>

struct ProgressCallback {
public:
	enum class LogLevel {
		error,
		warning,
		info,
		debug,
	};

	// Create a default progress. Log level is set to Info
	ProgressCallback();
	// Create a progress and set logging level to @logLevel
	ProgressCallback(LogLevel logLevel);
	~ProgressCallback();

	// Write a message in the specified layer
	void debug(const char *format, ...);
	void info(const char *format, ...);
	void error(const char *format, ...);
	void warning(const char *format, ...);

	// Set logging level to @loggingLevel
	void setLogLevel(LogLevel loggingLevel);
private:
	static std::string getTimestamp();
	//Parameters:
	LogLevel logLevel;
};
