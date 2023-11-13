#ifndef _HTTP_H
#define _HTTP_H

#include "common_includes.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <microhttpd.h>
#include "trie.h"

#define LISTEN_PORT 8080
typedef enum {
    METHOD_GET,
    METHOD_PUT,
    METHOD_POST,
    METHOD_DELETE
} method_t;

extern bool start_server( void );
extern bool stop_server( void );
extern bool register_route( char *, method_t, void *);
extern void * remove_route( char *, method_t );
#endif // _HTTP_D
