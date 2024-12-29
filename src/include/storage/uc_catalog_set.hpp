//===----------------------------------------------------------------------===//
//                         DuckDB
//
// storage/uc_catalog_set.hpp
//
//
//===----------------------------------------------------------------------===//

#pragma once

#include "duckdb/transaction/transaction.hpp"
#include "duckdb/common/case_insensitive_map.hpp"
#include "duckdb/common/mutex.hpp"

namespace duckdb {
struct DropInfo;
class UCSchemaEntry;
class UCTransaction;

class UCCatalogSet {
public:
	UCCatalogSet(Catalog &catalog);

	optional_ptr<CatalogEntry> GetEntry(ClientContext &context, const string &name);
	virtual void DropEntry(ClientContext &context, DropInfo &info);
	void Scan(ClientContext &context, const std::function<void(CatalogEntry &)> &callback);
	virtual optional_ptr<CatalogEntry> CreateEntry(unique_ptr<CatalogEntry> entry);
	void ClearEntries();

protected:
	virtual void LoadEntries(ClientContext &context) = 0;

	void EraseEntryInternal(const string &name);

protected:
	Catalog &catalog;

private:
	mutex entry_lock;
	case_insensitive_map_t<unique_ptr<CatalogEntry>> entries;
	bool is_loaded;
};

class UCInSchemaSet : public UCCatalogSet {
public:
	UCInSchemaSet(UCSchemaEntry &schema);

	optional_ptr<CatalogEntry> CreateEntry(unique_ptr<CatalogEntry> entry) override;

protected:
	UCSchemaEntry &schema;
};

} // namespace duckdb
