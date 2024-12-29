# This file is included by DuckDB's build system. It specifies which extension to load

# Extension from this repo
duckdb_extension_load(polaris
    SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}
    LOAD_TESTS
)

# duckdb_extension_load(httpfs)
# duckdb_extension_load(iceberg
#        GIT_URL https://github.com/duckdb/duckdb_iceberg
#        GIT_TAG main
# )
