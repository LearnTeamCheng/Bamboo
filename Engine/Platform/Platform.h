#pragma once

#if  defined(_WIN32) || define(_WIN64)
	#define BAMBOO_PLATFORM_WINDOWS	
	#include "Windows/PlatformWindows.h"
#else
#endif