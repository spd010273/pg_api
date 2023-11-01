#include "util.h"
static void _usage( char * );

static void _usage( char * message )
{
    return;
}

conninfo_t * _parse_args( int argc, char ** argv )
{
    int          c        = 0;
    char *       username = NULL;
    char *       hostname = NULL;
    char *       port     = NULL;
    char *       dbname   = NULL;
    uint16_t     port_val = 0;
    conninfo_t * result   = NULL;

    opterr = 0;

    while( ( c = getopt( argc, argv, "U:d:h:p:" ) ) != -1 )
    {
        switch( c )
        {
            case 'U':
                username = optarg;
                break;
            case 'd':
                dbname = optarg;
                break;
            case 'h':
                hostname = optarg;
                break;
            case 'p':
                port = optarg;
                break;
            default:
                _usage( "Invalid argument" );
        }
    }

    if( port == NULL )
    {
        port_val = 5432;
    }
    else
    {
        c = atoi( port );
        if( c <= 0 || c > 65535 )
            port_val = 5432;
        else
            port_val = ( uint16_t ) c;
    }

    if( username == NULL )
        username = "postgres"; 

    if( dbname == NULL )
        dbname = username;

    if( hostname == NULL )
        hostname = "localhost";

    result = ( conninfo_t * ) calloc(
        1,
        sizeof( conninfo_t )
    );

    if( result == NULL )
        return NULL;

    result->port = port_val;
    strncpy( result->username, username, MAX_CONNINFO_LEN );
    strncpy( result->hostname, hostname, MAX_CONNINFO_LEN );
    strncpy( result->dbname, dbname, MAX_CONNINFO_LEN );

    return result;
}
