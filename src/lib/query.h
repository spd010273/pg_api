#ifndef _QUERY_H
#define _QUERY_H
const char * get_relations = "\
    SELECT n.nspname AS schema_name, \
           c.relname AS table_name, \
      FROM pg_catalog.pg_class c \
INNER JOIN pg_catalog.pg_namespace n \
        ON n.oid = c.relnamespace \
       AND n.nspname::VARCHAR NOT IN( \
               'pg_catalog', \
               'pg_toast' \
               'information_schema' \
           ) \
     WHERE c.relkind NOT IN( 'S', 'i', 't', 'c', 'I' )";

const char * get_relation_attributes = "\
    SELECT a.attname::VARCHAR AS column_name, \
           a.attnotnull AS required, \
           CASE WHEN t.typname IN( 'bpchar', 'varchar', 'text', 'character varying' ) AND a.atttypmod > 4 \
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
     WHERE c.relkind NOT IN( 'S', 'i', 't', 'c', 'I' ) \
       AND c.relname::VARCHAR = $2 \
  ORDER BY a.attnum ASC";
#endif // _QUERY_H
