# Polarity Catalog Extension
This is a proof-of-concept extension demonstrating DuckDB connecting to the Polaris Catalog to read Iceberg tables using the [iceberg extension](https://github.com/duckdb/duckdb-iceberg).

You can try it out using DuckDB (>= v1.0.0) by running:

```SQL
INSTALL '/path/to/polaris.duckdb_extension';
INSTALL iceberg;
LOAD iceberg;
LOAD polaris;
CREATE SECRET (
	TYPE PC,
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
git clone ...
brew install ninja
GEN=Ninja make {debug/release}
```
