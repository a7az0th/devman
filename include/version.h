#pragma once

#define VERSION_MAJOR (1)
#define VERSION_MINOR (0)
#define VERSION_PATCH (0)

inline void printUsage() {
	printf("DEVMAN version %d.%02d.%02d (Developed by Alexander Soklev)\n", VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH);
}
