#pragma once

#include <string>

struct CharString {
public:
	CharString(): str("") {}
	CharString(const char* s): str(s) {}

	~CharString() {}

	const char* ptr() const { return str.c_str(); }
	int length() const { return int(str.length()); }
private:
	std::string str;
};
