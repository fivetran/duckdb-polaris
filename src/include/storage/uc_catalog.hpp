//===----------------------------------------------------------------------===//
//                         DuckDB
//
// storage/uc_catalog.hpp
//
//
//===----------------------------------------------------------------------===//

#pragma once

#include "duckdb/catalog/catalog.hpp"
#include "duckdb/function/table_function.hpp"
#include "duckdb/common/enums/access_mode.hpp"
#include "storage/uc_schema_set.hpp"

namespace duckdb {
class UCSchemaEntry;

struct UCCredentials {
	string endpoint;
	string client_id;
	string client_secret;
	// required to query s3 tables
	string aws_region;
	// Catalog generates the token using client id & secret
	string token;
};

class UCClearCacheFunction : public TableFunction {
public:
	UCClearCacheFunction();

	static void ClearCacheOnSetting(ClientContext &context, SetScope scope, Value &parameter);
};

class UCCatalog : public Catalog {
public:
	explicit UCCatalog(AttachedDatabase &db_p, const string &internal_name, AccessMode access_mode,
	                   UCCredentials credentials);
	~UCCatalog();

	string internal_name;
	AccessMode access_mode;
	UCCredentials credentials;

public:
	void Initialize(bool load_builtin) override;
	string GetCatalogType() override {
		return "polaris";
	}

	optional_ptr<CatalogEntry> CreateSchema(CatalogTransaction transaction, CreateSchemaInfo &info) override;

	void ScanSchemas(ClientContext &context, std::function<void(SchemaCatalogEntry &)> callback) override;

	optional_ptr<SchemaCatalogEntry> GetSchema(CatalogTransaction transaction, const string &schema_name,
	                                           OnEntryNotFound if_not_found,
	                                           QueryErrorContext error_context = QueryErrorContext()) override;

	unique_ptr<PhysicalOperator> PlanInsert(ClientContext &context, LogicalInsert &op,
	                                        unique_ptr<PhysicalOperator> plan) override;
	unique_ptr<PhysicalOperator> PlanCreateTableAs(ClientContext &context, LogicalCreateTable &op,
	                                               unique_ptr<PhysicalOperator> plan) override;
	unique_ptr<PhysicalOperator> PlanDelete(ClientContext &context, LogicalDelete &op,
	                                        unique_ptr<PhysicalOperator> plan) override;
	unique_ptr<PhysicalOperator> PlanUpdate(ClientContext &context, LogicalUpdate &op,
	                                        unique_ptr<PhysicalOperator> plan) override;
	unique_ptr<LogicalOperator> BindCreateIndex(Binder &binder, CreateStatement &stmt, TableCatalogEntry &table,
	                                            unique_ptr<LogicalOperator> plan) override;

	DatabaseSize GetDatabaseSize(ClientContext &context) override;

	//! Whether or not this is an in-memory PC database
	bool InMemory() override;
	string GetDBPath() override;

	void ClearCache();

private:
	void DropSchema(ClientContext &context, DropInfo &info) override;

private:
	UCSchemaSet schemas;
	string default_schema;
};

} // namespace duckdb
