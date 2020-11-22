#ifndef window_h
#define window_h

#ifdef __linux__
#include "xcb_window.h"
#elif _WIN32
#error "No support for Windows yet"
#elif __APPLE__
#error "No support for Mac OS X yet"
#endif

#endif
