#include "server.h"

int32_t main( int32_t argc, char ** argv )
{
    conninfo_t *  connection_info = NULL;
    PGconn *      conn            = NULL;
    uint32_t      num_rels        = 0;
    relation_t *  rels            = NULL;

    connection_info = _parse_args( argc, argv );

    if( connection_info == NULL )
        _log( LOG_LEVEL_FATAL, "Error parsing arguments" );

    _open_logs();

    conn = db_connect( NULL, connection_info );

    if( !CONN_GOOD( conn ) )
        _log( LOG_LEVEL_FATAL, "Failed to connect to database" );

    if( !enumerate_relations( conn, &rels, &num_rels ) )
        _log( LOG_LEVEL_FATAL, "Failed to enumerate relations" );

    PQfinish( conn );
    return 0;
}
