#ifndef _HTTP_H
#define _HTTP_H

#include "util.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <microhttpd.h>
#include "trie.h"
#include "pg.h"

#define LISTEN_PORT 8080
typedef enum {
    METHOD_GET,
    METHOD_PUT,
    METHOD_POST,
    METHOD_DELETE,
    METHOD_UNDEFINED
} method_t;

typedef struct {
    relation_t *   rel;
    method_t       method;
    route_arg_t ** arg;
    uint32_t       arglen;
    bool           slash;
} url_t;

extern bool start_server( conninfo_t * );
extern bool stop_server( void );
extern bool register_route( char *, method_t, void *);
extern void * remove_route( char *, method_t );
#endif // _HTTP_H
