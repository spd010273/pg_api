#include "query.h"

const char * get_relations = "\
    SELECT n.nspname AS schema_name, \
           c.relname AS table_name, \
           COALESCE( ta.table_alias, c.relname ) AS table_alias \
      FROM pg_catalog.pg_class c \
INNER JOIN pg_catalog.pg_namespace n \
        ON n.oid = c.relnamespace \
       AND n.nspname::VARCHAR NOT IN( \
               'pg_catalog', \
               'pg_toast', \
               'information_schema', \
               '" EXTENSION_NAME "' \
           ) \
 LEFT JOIN " EXTENSION_NAME ".tb_table_alias ta \
        ON ta.table_name = c.relname \
       AND ta.schema_name = n.nspname \
     WHERE c.relkind NOT IN( 'S', 'i', 't', 'c', 'I' )";

const char * get_relation_attributes = "\
    SELECT a.attname::VARCHAR AS column_name, \
           NULLIF( a.attnotnull, FALSE ) AS required, \
           co_p.oid AS primary, \
           co_u.oid AS unique, \
           CASE WHEN t.typname IN( 'bpchar', 'varchar', 'text', 'character varying' ) \
                 AND a.atttypmod > 4 \
                THEN a.atttypmod - 4 \
                ELSE NULL \
                 END AS max_length, \
           t.typname AS column_type \
      FROM pg_catalog.pg_class c \
INNER JOIN pg_catalog.pg_namespace n \
        ON n.oid = c.relnamespace \
       AND n.nspname::VARCHAR NOT IN( \
               'pg_catalog', \
               'pg_toast', \
               'information_schema' \
           ) \
       AND n.nspname::VARCHAR = $1 \
INNER JOIN pg_catalog.pg_attribute a \
        ON a.attrelid = c.oid \
       AND a.attisdropped IS FALSE \
       AND a.attnum > 0 \
INNER JOIN pg_catalog.pg_type t \
        ON t.oid = a.atttypid \
 LEFT JOIN pg_catalog.pg_constraint co_p \
        ON co_p.conrelid = c.oid \
       AND co_p.contype = 'p' \
       AND a.attnum = ALL( co_p.conkey ) \
 LEFT JOIN pg_catalog.pg_constraint co_u \
        ON co_u.conrelid = c.oid \
       AND co_u.contype = 'u' \
       AND a.attnum = ANY( co_u.conkey ) \
     WHERE c.relkind NOT IN( 'S', 'i', 't', 'c', 'I' ) \
       AND c.relname::VARCHAR = $2 \
  ORDER BY a.attnum ASC";
