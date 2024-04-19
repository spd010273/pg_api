#include "trie.h"
static bool _trie_string_safety_check( char * );
static trie_t * _new_trie_node( void );
static void _trie_free( trie_t * );
static bool _trie_has_children( trie_t * );

static const char _trie_search_chars[TRIE_SIZE] = "\
 !\"#$%&'()*+,-.0123456789:;<=\
>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\
\\]^_`abcdefghijklmnopqrstuvwxy\
z{|}~'";

static bool _trie_string_safety_check( char * str )
{
    uint32_t i = 0;
    for( i = 0; i < strlen( str ); i++ )
    {
        if( memchr( _trie_search_chars, str[i], TRIE_SIZE ) == NULL )
            return false;
    }

    return true;
}

static trie_t * _new_trie_node( void )
{
    trie_t * new = NULL;
    
    new = ( trie_t * ) calloc( sizeof( trie_t ), 1 );

    if( new == NULL )
        return NULL;

    return new;
}

static void _trie_free( trie_t * trie )
{
    uint32_t i = 0;

    if( trie == NULL )
        return;

    for( i = 0; i < TRIE_SIZE; i++ )
    {
        if( trie->character[i] != NULL )
            _trie_free( trie->character[i] );
        trie->character[i] = NULL;
    }

    return;
}

static bool _trie_has_children( trie_t * t )
{
    uint32_t i = 0;

    if( t == NULL )
        return false;

    for( i = 0; i < TRIE_SIZE; i++ )
    {
        if( t->character[i] != NULL )
            return true;
    }

    return false;
}

bool trie_insert( trie_t ** head, char * str, void * data )
{
    trie_t *         curr = NULL;
    register uint8_t c    = 0;

    if( str == NULL || head == NULL )
        return false;

    if( !_trie_string_safety_check( str ) )
        return false;

    if( *head == NULL )
    {
        *head = _new_trie_node();
        if( *head == NULL )
            return false;
        return trie_insert( head, str, data );
    }

    curr = *head;

    while( *str )
    {
        c = *str - ' ';
        if( curr->character[c] == NULL )
        {
            curr->character[c] = _new_trie_node();
            if( curr->character[c] == NULL )
                return false;
        }

        curr = curr->character[c];

        if( curr == NULL )
            return false;
        str++;
    }

    curr->is_leaf = true;
    curr->data    = data;
    return true;
}

void * trie_search( trie_t * head, char * str )
{
    trie_t *         curr = NULL;
    register uint8_t c    = 0;

    if( head == NULL || str == NULL )
        return NULL;

    if( !_trie_string_safety_check( str ) )
        return NULL;
    
    curr = head;

    while( *str )
    {
        c = *str - ' ';

        if( curr->character[c] == NULL )
            return NULL;

        curr = curr->character[c];

        if( curr == NULL )
            return NULL;

        str++;
    }

    if( curr->is_leaf )
        return curr->data;
    return NULL;
}

void * trie_delete( trie_t ** head, char * str )
{
    trie_t *         curr = NULL;
    register uint8_t c    = 0;
    void *           data = NULL;

    if( head == NULL || *head == NULL || str == NULL )
        return NULL;
    curr = *head;

    c = *str - ' ';
    if( *str )
    {
        if(
               curr->character[c] != NULL
            && trie_delete( &(curr->character[c]), str + 1 )
            && curr->is_leaf == false
          )
        {

            if( !_trie_has_children( curr ) )
            {
                data       = curr->data;
                curr->data = NULL;
                _trie_free( curr );
                return data;
            }
        }
    }

    if( *str == '\0' && curr->is_leaf == false )
    {
        if( !_trie_has_children( curr ) )
        {
            data       = curr->data;
            curr->data = NULL;
            _trie_free( curr );
            return data;
        }

        curr->is_leaf = false;
        data = curr->data;
        curr->data = NULL;
        return data;
    }

    return NULL;
}
