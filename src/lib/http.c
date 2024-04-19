#include "http.h"

static const char * not_found = "<html><body>Not Found.</body></html>";
static struct MHD_Daemon * daemon_handle = NULL;
static trie_t * head = NULL;

static char * get_route_name( char *, method_t );
static char * get_route_name_ext( const char *, const char * );
enum MHD_Result parse_url_argument( void *, enum MHD_ValueKind, const char *, const char * );
static url_t * parse_url( const char *, const char *, struct MHD_Connection * );
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

static const char * _get_method_string( method_t );
static method_t _get_method( const char * );
static url_t * _new_url( void );
static void _free_url( url_t * );
static bool _url_add_argument( url_t *, const char *, column_t * );

// TODO - this proto can change for MHD version 0.9.64 where size_t is added to indicate valsize
// see https://www.gnu.org/software/libmicrohttpd/manual/html_node/microhttpd_002dcb.html
enum MHD_Result parse_url_argument( void * cls, enum MHD_ValueKind param_kind, const char * key, const char * val )
{
    url_t * urldata = NULL;
    uint32_t i = 0;
    uint32_t j = 0;

    if( cls == NULL )
        return MHD_NO;

    urldata = ( url_t * ) cls;

    if( key != NULL && val != NULL && strnlen( key, MAX_RELNAME_LEN ) > 0 && strlen( val ) > 0 )
    {
        for( i = 0; i < urldata->rel->num_columns; i++ )
        {
            if( strncmp( urldata->rel->columns[i].column_name, key, MAX_RELNAME_LEN ) == 0 )
            {
                // Check to see if we've already allocated a route_arg_t for this
                if( urldata->arglen > 0 )
                {
                    for( j = 0; j < urldata->arglen; j++ )
                    {
                        if( strncmp( urldata->arg[j]->column->column_name, key, MAX_RELNAME_LEN ) == 0 )
                        {
                            printf( "Duplicate request parameter '%s'\n", key );
                            return MHD_YES;
                        }
                    }
                }

                // If we're here we can alloc a new route_arg_t
                if( _url_add_argument( urldata, val, &(urldata->rel->columns[i]) ) )
                    return MHD_YES;

                return MHD_NO;
            }
        }
    }

    return MHD_YES;
}

static url_t * parse_url( const char * method, const char * url, struct MHD_Connection * connection )
{
    url_t *      result       = NULL;
    relation_t * resolved_rel = NULL;
    method_t     req_method   = METHOD_UNDEFINED;
    int32_t      arg_count    = 0;
    char *       param_start  = NULL;
    char *       route        = NULL;
    bool         slash        = false;

    if( ( method == NULL ) || ( url == NULL ) || ( connection == NULL ) )
    {
        _log( LOG_LEVEL_DEBUG, "parse_url: NULL args" );
        return NULL;
    }

    if( *url == '/' )
        url++;

    route = ( char * ) calloc( strnlen( url, MAX_RELNAME_LEN + MAX_PARAM_LEN ), sizeof( char ) );
    if( route == NULL )
    {
        _log( LOG_LEVEL_ERROR, "Failed to allocate memory" );
        return NULL;
    }

    strncpy( route, url, strnlen( url, MAX_RELNAME_LEN + MAX_PARAM_LEN ) );
    // Check - we may need to parse stuff like
    // GET route/<id> for simple deletes - so check for a trailing '/'
    param_start = memchr( url, '/', MIN( MAX_RELNAME_LEN + MAX_PARAM_LEN, strlen( url ) ) );

    if( param_start != NULL )
    {
        slash = true;
        route[param_start - url] = '\0';
        param_start++; // increment past '/'
    }

    resolved_rel = ( relation_t * ) trie_search( head, get_route_name_ext( route, method ) );

    free( route );
    if( resolved_rel == NULL )
    {
        _log( LOG_LEVEL_DEBUG, "Could not resolve relation for url %s", route );
        return NULL;
    }

    req_method = _get_method( method );

    if( req_method == METHOD_UNDEFINED )
    {
        _log( LOG_LEVEL_DEBUG, "Undefined method\n" );
        return NULL;
    }

    result = _new_url();

    if( result == NULL )
    {
        _log( LOG_LEVEL_ERROR, "Failed to allocate memory when url parsing" );
        return NULL;
    }

    result->method = req_method;
    result->rel    = resolved_rel;
    result->slash  = slash;

    arg_count = MHD_get_connection_values( connection, MHD_GET_ARGUMENT_KIND, NULL, NULL );

    if( arg_count > 0 )
    {
        printf( "Parsing %d arguments\n", arg_count );
        MHD_get_connection_values(
            connection,
            MHD_GET_ARGUMENT_KIND,
            parse_url_argument,
            ( void * ) result
        );
    }
    else if( slash && param_start != NULL )
    {
        if( !_url_add_argument( result, param_start, result->rel->primary ) )
        {
            _log(
                LOG_LEVEL_ERROR,
                "Bad route. slash parameter passed to %s with no primary key on relation %s.%s",
                result->rel->route_alias,
                result->rel->schema_name,
                result->rel->table_name
            );
            return NULL; 
        }
    }

    return result;
}

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
    struct MHD_Response * response     = NULL;
    enum MHD_Result       ret          = 0;
    url_t *               parsed_url   = NULL;
    relation_t *          resolved_rel = NULL;
    const char * result = "<html><body>Hello, World!</body></html>";
    uint32_t i = 0;

    parsed_url = parse_url( method, url, connection );

    if( parsed_url == NULL )
        goto r_404;

    resolved_rel = parsed_url->rel;

#ifdef DEBUG
    printf(
        "Got hit for relation %s.%s with url %s, method %s\n",
        resolved_rel->schema_name,
        resolved_rel->table_name,
        url,
        method
    );

    for( i = 0; i < parsed_url->arglen; i++ )
    {
        printf(
            "Arg[%u]: '%s' = '%s'\n",
            i,
            parsed_url->arg[i]->column->column_name,
            parsed_url->arg[i]->arg
        );
    }
#endif // DEBUG

    // Handle request here
    if( parsed_url->method == METHOD_GET )
    {
        if( parsed_url->slash && parsed_url->arglen == 1 )
        {
            if( parsed_url->arg[0]->column->is_primary )
            {
                // Simple get
            }
            else
            {
                // relation has no primary attribute and single get won't work
                _log(
                    LOG_LEVEL_ERROR,
                    "Bad GET to %s. Relation %s.%s lacks a surrogate primary key",
                    parsed_url->rel->route_alias,
                    parsed_url->rel->schema_name,
                    parsed_url->rel->table_name
                );
                _free_url( parsed_url );
                goto r_404;
            }
        }
        else if( !parsed_url->slash && parsed_url->arglen > 0 )
        {
            // Search
        }
        else
        {
            _log(
                LOG_LEVEL_ERROR,
                "Bad GET to %s. missing parameter or uri in incorrect format",
                parsed_url->rel->route_alias
            );
            _free_url( parsed_url );
            goto r_404;
        }
    }
    else if( parsed_url->method == METHOD_PUT )
    {
        // INSERT
    }
    else if( parsed_url->method == METHOD_POST ) // or PATCH
    {
        // UPDATE
    }
    else if( parsed_url->method == METHOD_DELETE )
    {
        // DELETE
    }


    response = MHD_create_response_from_buffer(
        strlen( result ),
        ( void * ) result,
        MHD_RESPMEM_PERSISTENT
    );

    _free_url( parsed_url );

    ret = MHD_queue_response( connection, MHD_HTTP_OK, response );
    MHD_destroy_response( response );

    return ret;
r_404:
    response = MHD_create_response_from_buffer(
        strlen( not_found ),
        ( void * ) not_found,
        MHD_RESPMEM_PERSISTENT
    );

    ret = MHD_queue_response( connection, 404, response );
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

static method_t _get_method( const char * method )
{
    if( method == NULL )
        return METHOD_UNDEFINED;

    if(
          strncmp( method, "get", MIN( 3, strlen( method ) ) ) == 0
       || strncmp( method, "GET", MIN( 3, strlen( method ) ) ) == 0
      )
        return METHOD_GET;

    if(
          strncmp( method, "put", MIN( 3, strlen( method ) ) ) == 0
       || strncmp( method, "PUT", MIN( 3, strlen( method ) ) ) == 0
      )
        return METHOD_PUT;

    if(
          strncmp( method, "post", MIN( 3, strlen( method ) ) ) == 0
       || strncmp( method, "POST", MIN( 3, strlen( method ) ) ) == 0
      )
        return METHOD_POST;

    if(
          strncmp( method, "delete", MIN( 3, strlen( method ) ) ) == 0
       || strncmp( method, "DELETE", MIN( 3, strlen( method ) ) ) == 0
      )
        return METHOD_DELETE;

    return METHOD_UNDEFINED;
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

static url_t * _new_url( void )
{
    url_t * result = NULL;

    result = ( url_t * ) malloc( sizeof( url_t ) );

    if( result == NULL )
        return NULL;

    result->slash  = false;
    result->rel    = NULL;
    result->method = METHOD_UNDEFINED;
    result->arg    = NULL;
    result->arglen = 0;

    return result;
}

static void _free_url( url_t * url )
{
    uint32_t i = 0;

    if( url == NULL )
        return;

    if( url->arg != NULL )
    {
        for( i = 0; i < url->arglen; i++ )
        {
            if( url->arg[i] != NULL )
            {
                free( url->arg[i] );
            }
        }

        free( url->arg );
        url->arg    = NULL;
        url->arglen = 0;
    }

    url->method = METHOD_UNDEFINED;
    url->rel    = NULL;
    free( url );
    return;
}

//DEVNOTE: We can add datatype checking - cursory for now, or lean heavily on libpq / parameterization on the SQL side
static bool _url_add_argument( url_t * urldata, const char * val, column_t * column )
{
    uint32_t i = 0;

    if( column == NULL || val == NULL || urldata == NULL )
        return false;

    if( urldata->arglen == 0 )
    {
        i = 0;
        urldata->arglen = 1;
        urldata->arg    = ( route_arg_t ** ) malloc( sizeof( route_arg_t * ) );

        if( urldata->arg == NULL )
            return false;
    }
    else
    {
        i = urldata->arglen;
        urldata->arglen = i + 1;
        urldata->arg    = ( route_arg_t ** ) realloc(
            ( void * ) urldata->arg,
            sizeof( route_arg_t * ) * ( i + 1 )
        );

        if( urldata->arg == NULL )
            return false;
    }

    urldata->arg[i] = ( route_arg_t * ) calloc( 1, sizeof( route_arg_t ) );

    if( urldata->arg[i] == NULL )
    {
        free( urldata->arg );
        return false;
    }

    urldata->arg[i]->column = column;
    urldata->arg[i]->arg    = ( char * ) calloc(
        strlen( val ) + 1,
        sizeof( char )
    );

    if( urldata->arg[i]->arg == NULL )
        return false;
    strcpy( urldata->arg[i]->arg, val );
    return true;
}
