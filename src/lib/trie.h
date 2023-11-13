#ifndef _TRIE_H
#define _TRIE_H

#include "common_includes.h"

#define TRIE_SIZE 96

typedef struct trie {
    void *        data;
    struct trie * character[TRIE_SIZE];
    bool          is_leaf;
} trie_t;

bool trie_insert( trie_t **, char *, void * );
void * trie_search( trie_t *, char * );
void * trie_delete( trie_t **, char * );
#endif // _TRIE_H
