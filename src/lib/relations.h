#ifndef _RELATIONS_H
#define _RELATIONS_H

#include "common_includes.h"
#include "pg.h"
#include "query.h"

void free_relations( relation_t *, uint32_t );
bool enumerate_relations( PGconn *, relation_t **, uint32_t * );
//PGResult * get_tuple_by_id();
#endif // _RELATIONS_H
