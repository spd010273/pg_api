#ifndef _UTIL_H
#define _UTIL_H

#include "common_includes.h"
#include <stdarg.h>

#define LOG_LEVEL_DEBUG 1
#define LOG_LEVEL_INFO 2
#define LOG_LEVEL_WARNING 3
#define LOG_LEVEL_ERROR 4
#define LOG_LEVEL_FATAL 5

conninfo_t * _parse_args( int32_t argc, char ** argv );

#endif // _UTIL_H
