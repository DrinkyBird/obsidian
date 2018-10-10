#ifndef __PLATFORM_H__
#define __PLATFORM_H__

#ifdef _WIN32
#include "win32/win_platform.h"
#elif defined(__unix__)
#include "posix/posix_platform.h"
#endif

#endif