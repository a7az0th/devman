#pragma once

#include "charstring.h"
#include <assert.h>

struct ErrorCode {
public:
	ErrorCode(): errorFlag(0), errorCode(0) {}
	ErrorCode(const char* functionName, const int errorCode, const CharString &message): errorFlag(1), functionName(functionName), errorMessage(message), errorCode(errorCode) {
		assert(false);
	}

	CharString getError() {
		char str[256];
		sprintf(str, "%s: %s (%d)", functionName.ptr(), errorMessage.ptr(), errorCode);
		return CharString(str);
	}
	~ErrorCode() {}
	bool error() { return errorFlag; }

private:
	//Parameters:
	int errorCode;
	bool errorFlag;
	CharString functionName;
	CharString errorMessage;
};
