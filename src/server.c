#include "lib/common_includes.h"
#include "lib/http.h"
#include "lib/util.h"
#include "lib/pg.h"

int32_t main( int32_t, char ** );

int32_t main( int32_t argc, char ** argv )
{
    conninfo_t * connection_info = NULL;
    char * conninfo = NULL;
    PGconn *     conn = NULL;

    connection_info = _parse_args( argc, argv );

    if( connection_info == NULL )
        return 1;

    conninfo = get_connection_string( connection_info );
    _log( LOG_LEVEL_INFO, "%s", conninfo );
}
