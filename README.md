# Polarity Catalog Extension
This is a proof-of-concept extension demonstrating DuckDB connecting to the Polaris Catalog to read Iceberg tables using the [iceberg extension](https://github.com/duckdb/duckdb-iceberg).

You can try it out using DuckDB (>= v1.1.3) by doing the following:
```bash
DYLD_INSERT_LIBRARIES=/Library/Developer/CommandLineTools/usr/lib/clang/16/lib/darwin/libclang_rt.asan_osx_dynamic.dylib duckdb --unsigned
```

```SQL
INSTALL '/path/to/polaris.duckdb_extension';
INSTALL iceberg;
INSTALL httpfs;
LOAD '/path/to/polaris.duckdb_extension';
LOAD iceberg;
LOAD httpfs;
CREATE SECRET (
	TYPE POLARIS,
	CLIENT_ID '${PC_CLIENT_ID}',
	CLIENT_SECRET '${PC_CLIENT_SECRET}',
	ENDPOINT '${PC_ENDPOINT}',
	AWS_REGION '${PC_AWS_REGION}'
)
ATTACH 'test_catalog' AS test_catalog (TYPE POLARIS)
SHOW ALL TABLES;
SELECT * FROM test_catalog.test_schema.test_table;
```

# How to build from source

```
git clone https://github.com/fivetran/duckdb-polaris.git
git submodule update --init --recursive
brew install ninja
GEN=Ninja make {debug/release}
```
