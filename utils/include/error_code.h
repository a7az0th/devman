#pragma once

#include <string>
#include <assert.h>

struct ErrorCode {
public:
	ErrorCode(): errorFlag(0), errorCode(0) {}
	ErrorCode(const char* functionNamePtr, const int errorCode, const char* message): 
		errorFlag(1), errorCode(errorCode) 
	{
		if (functionNamePtr) {
			functionName = functionNamePtr; 
		}
		if (message) {
			errorMessage = message;
		}
		//assert(false);
	}

	std::string getError() {
		char str[256];
		if (functionName.length() > 0) {
			sprintf(str, "%s: %s (Code: %d)", functionName.c_str(), errorMessage.c_str(), errorCode);
		} else {
			sprintf(str, "%s (Code: %d)", errorMessage.c_str(), errorCode);
		}
		return std::string(str);
	}
	~ErrorCode() {}
	bool error() { return errorFlag; }

private:
	//Parameters:
	int errorCode;
	bool errorFlag;
	std::string functionName;
	std::string errorMessage;
};
