#include "relations.h"

void free_relations( relation_t * relations, uint32_t num_relations )
{
    uint32_t i = 0;

    if( num_relations > 0 )
    {
        for( i = 0; i < num_relations; i++ )
        {
            if( relations[i].columns != NULL )
            {
                memset( relations[i].columns, 0, sizeof( column_t ) * relations[i].num_columns );
                free( relations[i].columns );
            }
        }
    }

    if( relations != NULL )
    {
        memset( relations, 0, sizeof( relation_t ) * num_relations );
        free( relations );
    }

    return;
}

bool enumerate_relations( PGconn * conn, relation_t ** relations, uint32_t * num_relations )
{
    PGresult *            rel_result  = NULL;
    PGresult *            col_result  = NULL;
    uint32_t              i           = 0;
    uint32_t              j           = 0;
    uint32_t              num_columns = 0;
    char *                params[2]   = {NULL};
    register relation_t * rel         = NULL;
    register column_t   * col         = NULL;

    if( !CONN_GOOD( conn ) || relations == NULL || num_relations == NULL )
        return false;

    rel_result = execute_query( conn, ( char * ) get_relations, NULL, 0 );

    if( rel_result == NULL )
        return false;

    *num_relations = PQntuples( rel_result );

    *relations = ( relation_t * ) calloc(
        sizeof( relation_t ),
        PQntuples( rel_result )
    );

    if( *relations == NULL )
        return false;

    for( i = 0; i < PQntuples( rel_result ); i++ )
    {
        rel = &((*relations)[i]);
        params[0] = get_column_value( i, rel_result, "schema_name" );
        params[1] = get_column_value( i, rel_result, "table_name"  );

        strncpy( rel->schema_name, params[0], MAX_RELNAME_LEN );
        strncpy( rel->table_name, params[1], MAX_RELNAME_LEN );
        strncpy( rel->route_alias, get_column_value( i, rel_result, "table_alias" ), MAX_RELNAME_LEN );
        col_result = execute_query( conn, ( char * ) get_relation_attributes, params, 2 );

        if( col_result == NULL )
        {
            PQclear( rel_result );
            free_relations( *relations, *num_relations );
            return false;
        }

        num_columns = PQntuples( col_result );
        rel->num_columns = num_columns;
        rel->can_write   = true;
        rel->can_read    = true;
        rel->columns     = ( column_t * ) calloc(
            sizeof( column_t ),
            num_columns
        );

        if( rel->columns == NULL )
        {
            PQclear( col_result );
            PQclear( rel_result );
            free_relations( *relations, *num_relations );
            return false;
        }
        for( j = 0; j < num_columns; j++ )
        {
            col = &(rel->columns[j]);
            strncpy(
                col->column_name,
                get_column_value( j, col_result, "column_name" ),
                MAX_RELNAME_LEN
            );

            if( is_column_null( j, col_result, "max_length" ) )
            {
                strncpy(
                    col->data_type,
                    get_column_value( j, col_result, "column_type" ),
                    MAX_RELNAME_LEN
                );
            }
            else
            {
                snprintf(
                    col->data_type,
                    MAX_RELNAME_LEN,
                    "%s(%s)",
                    get_column_value( j, col_result, "column_type" ),
                    get_column_value( j, col_result, "max_length" )
                );
            }

            col->is_not_null = true;

            if( is_column_null( j, col_result, "required" ) )
                col->is_not_null = false;
        }
    }

    return true;
}

