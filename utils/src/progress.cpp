#include "progress.h"


void ProgressCallback::setLogLevel(LogLevel loggingLevel) {
	this->logLevel = loggingLevel;
}

ProgressCallback::ProgressCallback(): 
	logLevel(LogLevel::info) 
{
	//blank
}

ProgressCallback::ProgressCallback(LogLevel logLevel):
	logLevel(logLevel)
{
	//blank
}

void ProgressCallback::debug(const char *format, ...) {
	va_list args;
	va_start(args, format);

	if(logLevel >= LogLevel::debug) {
		printf("[DEBUG] ");
		vprintf(format, args);
		printf("\n");
	}

	va_end(args);
}

void ProgressCallback::info(const char *format, ...) {
	va_list args;
	va_start(args, format);

	if(logLevel >= LogLevel::info) {
		printf("[INFO] ");
		vprintf(format, args);
		printf("\n");
	}

	va_end(args);
}

void ProgressCallback::error(const char *format, ...) {
	va_list args;
	va_start(args, format);

	if(logLevel >= LogLevel::error) {
		printf("[ERROR] ");
		vprintf(format, args);
		printf("\n");
	}

	va_end(args);
}

void ProgressCallback::warning(const char *format, ...) {
	va_list args;
	va_start(args, format);

	if(logLevel >= LogLevel::warning) {
		printf("[WARNING] ");
		vprintf(format, args);
		printf("\n");
	}

	va_end(args);

}

ProgressCallback::~ProgressCallback() {
	//blank
}
