#ifndef _COMMON_INCLUDES_H
#define _COMMON_INCLUDES_H
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>

#define LOG_FILE_NAME "/var/log/pg_api/pg_api.log"
#define MAX_CONNINFO_LEN 64

typedef struct {
    uint16_t port;
    char     hostname[MAX_CONNINFO_LEN + 1];
    char     username[MAX_CONNINFO_LEN + 1];
    char     dbname[MAX_CONNINFO_LEN + 1];
} conninfo_t;

#endif // _COMMON_INCLUDES_H
