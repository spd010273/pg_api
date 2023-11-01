#ifndef _PG_H
#define _PG_H
#include "common_includes.h"
#include <libpq-fe.h>

#define CONN_GOOD(x) ( x != NULL && PQstatus( x ) == CONNECTION_OK )

#define MAX_CONN_RETRIES 5

#define SQL_STATE_TERMINATED_BY_ADMINISTRATOR "57P01"
#define SQL_STATE_CANCELED_BY_ADMINISTRATOR "57014"
#define SQL_STATE_CONNECTION_FAILURE "08006"
#define SQL_STATE_SQLCLIENT_UNABLE_TO_ESTABLISH_SQLCONNECTION "08001"
#define SQL_STATE_CONNECTION_DOES_NOT_EXIST "08003"
#define SQL_STATE_CONNECTION_EXCEPTION "08000"


extern PGresult * execute_query( PGconn *, char *, char **, uint32_t );
extern PGconn * db_connect( PGconn *, conninfo_t * );
extern char * get_connection_string( conninfo_t * );
extern char * get_column_value( int32_t, PGresult *, char * );
extern bool is_column_null( int32_t, PGresult *, char * );
#endif // _PG_H

