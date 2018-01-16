// Another Reddit Console Client
// Copyright (c) 2017-2018, Adalid Claure <aclaure@gmail.com>

#pragma once

#ifdef _WINDOWS
#pragma warning(disable:4530)
#pragma warning(disable:4068)
#endif

#define __STRINGIFY__(x)        #x
#define __EXPAND__(x)           __STRINGIFY__(x)

#define VERSION_MAJOR			0
#define	VERSION_MINOR			0
#define VERSION_BUILD			1
#define VERSION					__EXPAND__(VERSION_MAJOR) "." __EXPAND__(VERSION_MINOR) "." __EXPAND__(VERSION_BUILD)
#define BUILDTIMESTAMP	        __DATE__ " " __TIME__

#define APP_NAME                "arcc"
#define APP_TITLE               APP_NAME " " VERSION
#define COPYRIGHT               "Copyright (c) 2017-2018, Adalid Claure <aclaure@gmail.com>"
