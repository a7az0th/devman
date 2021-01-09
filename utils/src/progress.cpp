#include "progress.h"
#include <chrono>

#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"

using namespace a7az0th;

std::string ProgressCallback::getTimestamp() {

	auto now = std::chrono::system_clock::now();
	auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();

	time_t currentTime = std::chrono::system_clock::to_time_t(now);

	const int Millis = ms % 1000;

	struct tm* localTime;
	localTime = localtime(&currentTime); // Convert the current time to the local time

	//[2016/01/08-23:12:04.43]
	const int Day    = localTime->tm_mday;
	const int Month  = localTime->tm_mon + 1;
	const int Year   = localTime->tm_year + 1900;
	const int Hour   = localTime->tm_hour;
	const int Min    = localTime->tm_min;
	const int Sec    = localTime->tm_sec;

	char fileName[256];
	sprintf(fileName, "%04d/%02d/%02d-%02d:%02d:%02d.%03d", Year, Month, Day, Hour, Min, Sec, Millis);
	return fileName;
}

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

void ProgressCallback::debug(const char *format, ...) const {

	if(logLevel >= LogLevel::debug) {
		va_list args;
		va_start(args, format);

		std::string timeStamp = ProgressCallback::getTimestamp();
		fprintf(stderr, "[%s]" ANSI_COLOR_MAGENTA "[  DEBUG] ", timeStamp.c_str());
		vfprintf(stderr, format, args);
		fprintf(stderr, ANSI_COLOR_RESET "\n");

		va_end(args);
	}
}

void ProgressCallback::info(const char *format, ...) const {
	if(logLevel >= LogLevel::info) {
		va_list args;
		va_start(args, format);

		std::string timeStamp = ProgressCallback::getTimestamp();
		fprintf(stderr, "[%s]"  "[   INFO] ", timeStamp.c_str());
		vfprintf(stderr, format, args);
		fprintf(stderr,  "\n");

		va_end(args);
	}
}

void ProgressCallback::error(const char *format, ...) const {
	if(logLevel >= LogLevel::error) {
		va_list args;
		va_start(args, format);

		std::string timeStamp = ProgressCallback::getTimestamp();
		fprintf(stderr, "[%s]" ANSI_COLOR_RED "[  ERROR] ", timeStamp.c_str());
		vfprintf(stderr, format, args);
		fprintf(stderr, ANSI_COLOR_RESET "\n");

		va_end(args);
	}
}

void ProgressCallback::warning(const char *format, ...) const {
	if(logLevel >= LogLevel::warning) {
		va_list args;
		va_start(args, format);

		std::string timeStamp = ProgressCallback::getTimestamp();
		fprintf(stderr, "[%s]" ANSI_COLOR_YELLOW "[WARNING] ", timeStamp.c_str());
		vfprintf(stderr, format, args);
		fprintf(stderr, ANSI_COLOR_RESET "\n");

		va_end(args);
	}
}

ProgressCallback::~ProgressCallback() {
	//blank
}
