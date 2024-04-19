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
#define MAX_RELNAME_LEN 64
#define MAX_PARAM_LEN 10 // Includes '/' + digits/id length
#define MIN(x,y) ( x < y ? x : y )
typedef struct {
    uint16_t port;
    char     hostname[MAX_CONNINFO_LEN + 1];
    char     username[MAX_CONNINFO_LEN + 1];
    char     dbname[MAX_CONNINFO_LEN + 1];
} conninfo_t;

typedef struct {
    char column_name[MAX_RELNAME_LEN + 1];
    char data_type[MAX_RELNAME_LEN + 1];
    bool is_not_null;
    bool is_primary;
    bool is_unique;
} column_t;

typedef struct {
    char       schema_name[MAX_RELNAME_LEN + 1];
    char       table_name[MAX_RELNAME_LEN + 1];
    char       route_alias[MAX_RELNAME_LEN + 1];
    bool       can_write;
    bool       can_read;
    uint16_t   num_columns;
    column_t * columns;
    column_t * primary;
} relation_t;
/*
typedef struct {
    error_type_t error_type;
    char * additional_info;
} error_t;
*/
typedef struct {
    column_t * column;
    char * arg;
} route_arg_t;

#endif // _COMMON_INCLUDES_H
