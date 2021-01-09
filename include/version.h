#pragma once
#include "progress.h"
#define VERSION_MAJOR (1)
#define VERSION_MINOR (0)
#define VERSION_PATCH (0)

inline void printVersion(const a7az0th::ProgressCallback &prog) {
	prog.info("DEVMAN version %d.%02d.%02d (Developed by Alexander Soklev)", VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH);
}
