#include "util.h"

static FILE * log_file   = NULL;
static pid_t  parent_pid = 0;
static bool   daemonize  = false;

static void _usage( char * ) __attribute__ ((noreturn));

static const char * usage_string = "\
Usage: pg_api\n \
    -U <db_user>\n \
    -d <db name>\n \
    -p <db port>\n \
    -h <db host>\n \
";

static void _usage( char * message )
{
    if( message != NULL )
        printf( "%s\n", message );

    printf( "%s", usage_string );
    exit( 1 );
}

void _terminate( void )
{
    exit( 1 );
}

void _logrotate( void )
{
    int32_t save_errno = 0;

    if( daemonize == false || log_file == NULL )
        return;

    save_errno = errno;
    fclose( log_file );
    log_file = NULL;
    errno = 0;
    log_file = fopen( LOG_FILE_NAME, "a" );
    
    if( log_file == NULL )
    {
        _log(
            LOG_LEVEL_ERROR,
            "Failed to rotate log file '%s': %s",
            LOG_FILE_NAME,
            strerror( errno )
        );
    }

    errno = save_errno;

    return;
}

void _open_logs( void )
{
    int32_t save_errno = 0;

    if( log_file != NULL || daemonize == false )
        return;

    save_errno = errno;

    log_file = fopen( LOG_FILE_NAME, "a" );

    if( log_file == NULL )
    {
        _log(
            LOG_LEVEL_ERROR,
            "Failed to open log file '%s': %s",
            LOG_FILE_NAME,
            strerror( errno )
        );
    }

    parent_pid = getpid();
    errno = save_errno;
    return;
}

void _log( uint8_t log_level, char * message, ... )
{
    va_list        args                           = {{0}};
    FILE *         output_handle                  = NULL;
    struct timeval tv                             = {0};
    char           time_buff[MAX_TIME_PREFIX + 1] = {0};
    char *         log_prefix                     = NULL;

#ifndef DEBUG
    if( log_level == LOG_LEVEL_DEBUG )
        return;
#endif // DEBUG

    if( message == NULL )
        return;

    if     ( log_level == LOG_LEVEL_DEBUG   ) { log_prefix = "DEBUG";   }
    else if( log_level == LOG_LEVEL_INFO    ) { log_prefix = "INFO";    }
    else if( log_level == LOG_LEVEL_WARNING ) { log_prefix = "WARNING"; }
    else if( log_level == LOG_LEVEL_ERROR   ) { log_prefix = "ERROR";   }
    else if( log_level == LOG_LEVEL_FATAL   ) { log_prefix = "FATAL";   }
    else                                      { log_prefix = "UNKNOWN"; }

    gettimeofday( &tv, NULL );
    strftime(
        time_buff,
        MAX_TIME_PREFIX,
        "%Y-%m-%d %H:%M:%S",
        gmtime( &tv.tv_sec )
    );

    output_handle = stdout;

    if(
          log_level == LOG_LEVEL_WARNING
       || log_level == LOG_LEVEL_ERROR
       || log_level == LOG_LEVEL_FATAL
      )
    {
        output_handle = stderr;
    }

    if( log_file != NULL )
        output_handle = log_file;

    va_start( args, message );

    fprintf(
        output_handle,
        "%s.%05d [%d] %s: ",
        time_buff,
        ( int32_t ) ( tv.tv_usec / 1000 ),
        getpid(),
        log_prefix
    );

    vfprintf(
        output_handle,
        message,
        args
    );

    fprintf(
        output_handle,
        "\n"
    );

    va_end( args );
    fflush( output_handle );

    if( log_level == LOG_LEVEL_FATAL )
        _terminate();

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
