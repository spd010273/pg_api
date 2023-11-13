CREATE TABLE @extschema@.tb_table_alias
(
    schema_name VARCHAR(63),
    table_name  VARCHAR(63),
    table_alias VARCHAR(63),
    UNIQUE( schema_name, table_name ),
    UNIQUE( table_alias )
);

GRANT USAGE ON SCHEMA @extschema@ TO PUBLIC;
GRANT SELECT, UPDATE, DELETE, INSERT ON ALL TABLES IN SCHEMA @extschema@ TO PUBLIC;
SELECT pg_catalog.pg_extension_config_dump( '@extschema@.tb_table_alias', '' );
