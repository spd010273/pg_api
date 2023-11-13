#include "server.h"

int32_t main( int32_t argc, char ** argv )
{
    conninfo_t *  connection_info = NULL;
    PGconn *      conn            = NULL;
    uint32_t      num_rels        = 0;
    relation_t *  rels            = NULL;
    uint32_t      i               = 0;

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

    if( num_rels <= 0 )
    {
        _log( LOG_LEVEL_INFO, "No routes to register" );
        return 0;
    }

    for( i = 0; i < num_rels; i++ )
    {
        if( rels[i].can_write )
        {
            _log( LOG_LEVEL_DEBUG, "Registering %s", rels[i].route_alias );
            if(
                   register_route( rels[i].route_alias, METHOD_DELETE, ( void * ) &(rels[i]) )
                && register_route( rels[i].route_alias, METHOD_PUT,    ( void * ) &(rels[i]) )
                && register_route( rels[i].route_alias, METHOD_POST,   ( void * ) &(rels[i]) )
              )
            {
                _log( LOG_LEVEL_DEBUG, "Write routes for %s registered", rels[i].table_name );
            }
            else
            {
                _log( LOG_LEVEL_ERROR, "Failed to register write routes for %s", rels[i].table_name );
            }
        }

        if( rels[i].can_read )
        {
            if( !register_route( rels[i].route_alias, METHOD_GET, ( void * ) &(rels[i]) ) )
            {
                _log( LOG_LEVEL_ERROR, "Failed to register read route for %s", rels[i].table_name );
            }
        }
    }

    start_server();
    while( 1 )
    {

    }
    stop_server();
    return 0;
}
