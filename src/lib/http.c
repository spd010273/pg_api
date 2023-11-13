#include "http.h"

static const char * not_found = "<html><body>Not Found.</body></html>";
static struct MHD_Daemon * daemon_handle = NULL;
static trie_t * head = NULL;

static char * get_route_name( char *, method_t );
static char * get_route_name_ext( const char *, const char * );
static const char * _get_method_string( method_t );
static enum MHD_Result router(
    void *,
    struct MHD_Connection *,
    const char *,
    const char *,
    const char *,
    const char *,
    size_t *,
    void **
);

static enum MHD_Result router(
    void *                  cls,
    struct MHD_Connection * connection,
    const char *            url,
    const char *            method,
    const char *            version,
    const char *            upload_data,
    size_t *                upload_size,
    void **                 con_cls
)
{
    const char * result = "<html><body>Hello, World!</body></html>";
    struct MHD_Response * response = NULL;
    enum MHD_Result ret = 0;
    relation_t * resolved_rel = NULL;

    resolved_rel = ( relation_t * ) trie_search( head, get_route_name_ext( url, method ) );
  
    if( resolved_rel == NULL )
    {
        response = MHD_create_response_from_buffer(
            strlen( not_found ),
            ( void * ) not_found,
            MHD_RESPMEM_PERSISTENT
        );

        ret = MHD_queue_response( connection, 404, response );
        MHD_destroy_response( response );
        return ret;
    }

    printf(
        "Got hit for relation %s.%s\n",
        resolved_rel->schema_name,
        resolved_rel->table_name
    );
    response = MHD_create_response_from_buffer(
        strlen( result ),
        ( void * ) result,
        MHD_RESPMEM_PERSISTENT
    );

    ret = MHD_queue_response( connection, MHD_HTTP_OK, response );
    MHD_destroy_response( response );

    return ret;
}

bool start_server( void )
{
    if( daemon_handle == NULL )
    {
        daemon_handle = MHD_start_daemon(
            MHD_USE_AUTO | MHD_USE_INTERNAL_POLLING_THREAD,
            LISTEN_PORT,
            NULL,
            NULL,
            &router,
            NULL,
            MHD_OPTION_END
        );

        if( daemon_handle == NULL )
            return false;
    }

    return true;
}

bool stop_server( void )
{
    if( daemon_handle != NULL )
    {
        MHD_stop_daemon( daemon_handle );
        daemon_handle = NULL;
    }

    return true;
}

static const char * _get_method_string( method_t method )
{
    switch( method )
    {
        case METHOD_GET:
            return "GET";
        case METHOD_PUT:
            return "PUT";
        case METHOD_POST:
            return "POST";
        case METHOD_DELETE:
            return "DELETE";
        default:
            return NULL;
    }

    return NULL;
}

static char * get_route_name_ext( const char * route, const char * method )
{
    char * result   = NULL;
    size_t res_size = 0;

    if( route == NULL || method == NULL )
        return NULL;

    if( *route == '/' )
        route++;

    res_size = snprintf( NULL, 0, "%s#%s", route, method );
    result   = ( char * ) malloc( sizeof( char ) * ( res_size + 1 ) );

    if( result == NULL )
        return NULL;

    if( snprintf( result, res_size, "%s#%s", route, method ) < res_size )
    {
        free( result );
        return NULL;
    }

    result[res_size] = '\0';
    return result;
}

static char * get_route_name( char * route, method_t method )
{
    char * result   = NULL;
    size_t res_size = 0;

    if( route == NULL )
        return NULL;

    res_size = snprintf( NULL, 0, "%s#%s", route, _get_method_string( method ) );
    result   = ( char * ) malloc( sizeof( char ) * ( res_size + 1 ) );

    if( result == NULL )
        return NULL;

    if( snprintf( result, res_size, "%s#%s", route, _get_method_string( method ) ) < res_size )
    {
        free( result );
        return NULL;
    }

    result[res_size] = '\0';
    return result;
}

bool register_route( char * route_name, method_t method, void * data )
{
    char * route_full_name = NULL;
    route_full_name = get_route_name( route_name, method );
    if( !trie_insert( &head, route_full_name, data  ) )
    {
        free( route_full_name );
        return false;
    }

    return true;
}

void * remove_route( char * route_name, method_t method )
{
    void * data = NULL;
    data = trie_delete( &head, get_route_name( route_name, method ) );
    if( data == NULL )
        return NULL;
    return data;
}
