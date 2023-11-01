#include "pg.h"

char * get_connection_string( conninfo_t * conn_data )
{
    char *   conninfo = NULL;
    char *   port_str = NULL;
    uint32_t len      = 0;

    if( conn_data == NULL )
        return NULL;

    len = snprintf( NULL, 0, "%d", conn_data->port );
    port_str = ( char * ) calloc( len + 1, sizeof( char ) );

    if( port_str == NULL )
        return NULL;

    snprintf( port_str, len + 1, "%d", conn_data->port );
    len = strlen( conn_data->username ) + strlen( conn_data->dbname )
        + strlen( conn_data->hostname ) + strlen( port_str ) + 44;

    conninfo = ( char * ) calloc(
        len + 1,
        sizeof( char )
    );

    if( conninfo == NULL )
    {
        free( port_str );
        return NULL;
    }

    strcpy( conninfo, "user=" );
    strcat( conninfo, conn_data->username );
    strcat( conninfo, " host=" );
    strcat( conninfo, conn_data->hostname );
    strcat( conninfo, " port=" );
    strcat( conninfo, port_str );
    strcat( conninfo, " dbname=" );
    strcat( conninfo, conn_data->dbname );
    strcat( conninfo, " connect_timeout=1" );
    conninfo[len] = '\0';
    free( port_str );

    return conninfo;
}

PGconn * db_connect( PGconn * conn, conninfo_t * conninfo )
{
    uint32_t last_backoff_time = 1;
    uint32_t try_count         = 0;
    char * connection_string   = NULL;

    if( CONN_GOOD( conn ) )
        return conn;

    if( conninfo == NULL )
        return NULL;

    connection_string = get_connection_string( conninfo );

    if( connection_string == NULL )
        return NULL;
    
    conn = PQconnectdb( connection_string );

    while( !CONN_GOOD( conn ) && try_count < MAX_CONN_RETRIES )
    {
        if( conn != NULL )
            PQfinish( conn );

        conn = PQconnectdb( connection_string );
        try_count++;
        sleep( last_backoff_time );
        last_backoff_time = (int32_t) ( 10 * ( ( double ) rand() / ( double ) RAND_MAX ) )
                          + last_backoff_time;
    }

    if( !CONN_GOOD( conn ) )
    {
        if( conn != NULL )
            PQfinish( conn );
        return NULL;
    }

    return conn;
}

PGresult * execute_query( PGconn * conn, char * query, char ** params, uint32_t param_count )
{
    PGresult *   result              = NULL;
    char *       last_sql_state      = NULL;
    char *       temp_last_sql_state = NULL;
    uint32_t     retry_counter       = 0;
    uint32_t     last_backoff_time   = 1;

    if( !CONN_GOOD( conn ) )
         return NULL;

    while(
            (
                last_sql_state == NULL
             || strcmp(
                    last_sql_state,
                    SQL_STATE_TERMINATED_BY_ADMINISTRATOR
                ) == 0
             || strcmp(
                    last_sql_state,
                    SQL_STATE_CANCELED_BY_ADMINISTRATOR
                ) == 0
             || strcmp(
                    last_sql_state,
                    SQL_STATE_CONNECTION_FAILURE
                ) == 0
             || strcmp(
                    last_sql_state,
                    SQL_STATE_SQLCLIENT_UNABLE_TO_ESTABLISH_SQLCONNECTION
                ) == 0
             || strcmp(
                    last_sql_state,
                    SQL_STATE_CONNECTION_DOES_NOT_EXIST
                ) == 0
             || strcmp(
                    last_sql_state,
                    SQL_STATE_CONNECTION_EXCEPTION
                ) == 0
            )
         && retry_counter < MAX_CONN_RETRIES
        )
    {
        if( last_sql_state != NULL )
        {
            // We've made one pass, and our state is not null
            return NULL;
        }

        if( params == NULL )
        {
            result = PQexec( conn, query );
        }
        else
        {
            result = PQexecParams(
                conn,
                query,
                param_count,
                NULL,
                ( const char * const * ) params,
                NULL,
                NULL,
                0
            );
        }

        if(
                result != NULL
             && !(
                     PQresultStatus( result ) == PGRES_COMMAND_OK
                  || PQresultStatus( result ) == PGRES_TUPLES_OK
                 )
          )
        {

            temp_last_sql_state = PQresultErrorField(
                result,
                PG_DIAG_SQLSTATE
            );

            if( temp_last_sql_state != NULL )
            {
                if( last_sql_state != NULL )
                {
                    free( last_sql_state );
                    last_sql_state = NULL;
                }

                last_sql_state = ( char * ) calloc(
                    sizeof( char ),
                    strlen( temp_last_sql_state ) + 1
                );

                if( last_sql_state == NULL )
                    return NULL;

                strcpy( last_sql_state, temp_last_sql_state );

            }

            if( result != NULL )
                PQclear( result );

            retry_counter++;
            sleep( last_backoff_time );
            last_backoff_time = (int) ( 10 * ( ( double ) rand() / ( double ) RAND_MAX ) )
                              + last_backoff_time;
        }
        else
        {
            return result;
        }
    }

    if( last_sql_state != NULL )
    {
        free( last_sql_state );
        last_sql_state = NULL;
    }

    return NULL;
}

char * get_column_value( int32_t row, PGresult * result, char * column_name )
{
    if( is_column_null( row, result, column_name ) )
        return NULL;

    return PQgetvalue(
        result,
        row,
        PQfnumber(
            result,
            column_name
        )
    );
}

bool is_column_null( int32_t row, PGresult * result, char * column_name )
{
    if(
        PQgetisnull(
            result,
            row,
            PQfnumber(
                result,
                column_name
            )
        )
      )
    {
        return true;
    }

    return false;
}
