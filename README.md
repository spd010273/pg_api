pg_api
======

Program which generates a REST API from a PostgreSQL database, automatically.

## How does it work?

`pg_api` connects to the database server you specify, and enumerates all relations in non-restricted schemas as available routes. Depending on the connected user's permissions, the following routes will be generated:

- GET route - For retrieving a record by its primary key
- Search route - a pluralized GET, for returning multiple records
- POST route - For performing UPDATE operations on the table
- PUT route - For performing INSERT operations on the table
- DELETE route - For performing DELETE operations on the table

Optionally, users may install the PostgreSQL extension, which can be used to alias tables as they are exposed via HTTP. The following is an example for an arbitrary table called `tb_foobarbaz` with a primary key of `foobarbaz`

- `GET tb_foobarbaz/123`
- `GET tb_foobarbazs?bar=1&baz=5`
- `POST tb_foobarbaz`
- `PUT tb_foobarbaz`
- `DELETE tb_foobarbaz/123`

Aliasing could rename the exposed route from `tb_foobarbaz` to `foobarbaz` such that the routes become:

- `GET foobarbaz/123`
- `GET foobarbazs?bar=1&baz=5`
- `POST foobarbaz`
- `PUT foobarbaz`
- `DELETE foobarbaz/123`

This extension assumes that your relations have a surrogate primary key (colloquially: id) so that GET and DELETE routes can operate on a single, unique record. Relations failing this check will not work for these routes
## Requirements:

- libmicrohttpd-dev
- libpq-fe
