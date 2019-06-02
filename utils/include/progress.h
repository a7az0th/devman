#pragma once

#include <stdarg.h>
#include <string>

struct ProgressCallback {
public:
	enum class LogLevel {
		error,
		warning,
		info,
		debug,
	};


	ProgressCallback();
	ProgressCallback(LogLevel logLevel);
	~ProgressCallback();

	void debug(const char *format, ...);
	void info(const char *format, ...);
	void error(const char *format, ...);
	void warning(const char *format, ...);

	void setLogLevel(LogLevel loggingLevel);
private:
	//Parameters:
	LogLevel logLevel;
};
