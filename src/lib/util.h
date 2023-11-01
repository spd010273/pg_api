#ifndef _UTIL_H
#define _UTIL_H

#include "common_includes.h"
#include <stdarg.h>
#include <sys/time.h>
#include <time.h>
#include <errno.h>

#define LOG_LEVEL_DEBUG 1
#define LOG_LEVEL_INFO 2
#define LOG_LEVEL_WARNING 3
#define LOG_LEVEL_ERROR 4
#define LOG_LEVEL_FATAL 5

#define MAX_TIME_PREFIX 28

extern void _logrotate( void );
extern void _open_logs( void );
extern void _terminate( void ) __attribute__ ((noreturn));
extern void _log( uint8_t, char *, ... ) __attribute__ ((format (gnu_printf, 2, 3 )));
extern conninfo_t * _parse_args( int32_t argc, char ** argv );

#endif // _UTIL_H
