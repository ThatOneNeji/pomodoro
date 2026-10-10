/// @file version.cpp
#include "version.h"

// The only file that includes the generated header, so a version change recompiles just this one.
// Tools that don't run the PlatformIO pre-build script (editors, linters, cppcheck) get "unknown".
#if __has_include("version_generated.h")
#include "version_generated.h"
#else
#define FW_VERSION "unknown"
#define FW_BUILD_TIME "unknown"
#endif

const char *getFirmwareVersion() { return FW_VERSION; }

const char *getFirmwareBuildTime() { return FW_BUILD_TIME; }
