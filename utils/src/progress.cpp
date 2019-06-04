#include "progress.h"

std::string ProgressCallback::getTimestamp() {

	time_t currentTime;
	time( &currentTime ); // Get the current time

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
	sprintf(fileName, "%04d/%02d/%02d-%02d:%02d:%02d.00", Year, Month, Day, Hour, Min, Sec);
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

void ProgressCallback::debug(const char *format, ...) {
	va_list args;
	va_start(args, format);

	if(logLevel >= LogLevel::debug) {
		std::string timeStamp = ProgressCallback::getTimestamp();
		printf("[%s][DEBUG] ", timeStamp.c_str());
		vprintf(format, args);
		printf("\n");
	}

	va_end(args);
}

void ProgressCallback::info(const char *format, ...) {
	va_list args;
	va_start(args, format);

	if(logLevel >= LogLevel::info) {
		std::string timeStamp = ProgressCallback::getTimestamp();
		printf("[%s][INFO] ", timeStamp.c_str());
		vprintf(format, args);
		printf("\n");
	}

	va_end(args);
}

void ProgressCallback::error(const char *format, ...) {
	va_list args;
	va_start(args, format);

	if(logLevel >= LogLevel::error) {
		std::string timeStamp = ProgressCallback::getTimestamp();
		printf("[%s][ERROR] ", timeStamp.c_str());
		vprintf(format, args);
		printf("\n");
	}

	va_end(args);
}

void ProgressCallback::warning(const char *format, ...) {
	va_list args;
	va_start(args, format);

	if(logLevel >= LogLevel::warning) {
		std::string timeStamp = ProgressCallback::getTimestamp();
		printf("[%s][WARNING] ", timeStamp.c_str());
		vprintf(format, args);
		printf("\n");
	}

	va_end(args);

}

ProgressCallback::~ProgressCallback() {
	//blank
}
