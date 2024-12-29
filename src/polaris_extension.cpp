#define DUCKDB_EXTENSION_MAIN

#include "polaris_extension.hpp"
#include "storage/uc_catalog.hpp"
#include "storage/uc_transaction_manager.hpp"

#include "duckdb.hpp"
#include "duckdb/main/secret/secret_manager.hpp"
#include "duckdb/common/exception.hpp"
#include "duckdb/common/string_util.hpp"
#include "duckdb/function/scalar_function.hpp"
#include "duckdb/main/extension_util.hpp"
#include "duckdb/parser/parsed_data/create_scalar_function_info.hpp"
#include "duckdb/parser/parsed_data/attach_info.hpp"
#include "duckdb/storage/storage_extension.hpp"
#include "uc_api.hpp"

namespace duckdb {

static unique_ptr<BaseSecret> CreatePolarisSecretFunction(ClientContext &, CreateSecretInput &input) {
	// apply any overridden settings
	vector<string> prefix_paths;
	auto result = make_uniq<KeyValueSecret>(prefix_paths, "polaris", "config", input.name);
	
	for (const auto &named_param : input.options) {
		auto lower_name = StringUtil::Lower(named_param.first);

		if (lower_name == "client_id" || 
				lower_name == "client_secret" ||
				lower_name == "endpoint" ||
				lower_name == "aws_region") {
			result->secret_map[lower_name] = named_param.second.ToString();
		} else {
			throw InternalException("Unknown named parameter passed to CreateUCSecretFunction: " + lower_name);
		}
	}

	// Get token from catalog
	result->secret_map["token"] = UCAPI::GetToken(
		result->secret_map["client_id"].ToString(), 
		result->secret_map["client_secret"].ToString(),
		result->secret_map["endpoint"].ToString());
	
	//! Set redact keys
	result->redact_keys = {"token", "client_id", "client_secret"};

	return std::move(result);
}

static void SetPolarisSecretParameters(CreateSecretFunction &function) {
	function.named_parameters["client_id"] = LogicalType::VARCHAR;
	function.named_parameters["client_secret"] = LogicalType::VARCHAR;
	function.named_parameters["endpoint"] = LogicalType::VARCHAR;
	function.named_parameters["aws_region"] = LogicalType::VARCHAR;
	function.named_parameters["token"] = LogicalType::VARCHAR;
}

unique_ptr<SecretEntry> GetSecret(ClientContext &context, const string &secret_name) {
	auto &secret_manager = SecretManager::Get(context);
	auto transaction = CatalogTransaction::GetSystemCatalogTransaction(context);
	// FIXME: this should be adjusted once the `GetSecretByName` API supports this
	// use case
	auto secret_entry = secret_manager.GetSecretByName(transaction, secret_name, "memory");
	if (secret_entry) {
		return secret_entry;
	}
	secret_entry = secret_manager.GetSecretByName(transaction, secret_name, "local_file");
	if (secret_entry) {
		return secret_entry;
	}
	return nullptr;
}

static unique_ptr<Catalog> PolarisCatalogAttach(StorageExtensionInfo *storage_info, ClientContext &context,
                                           AttachedDatabase &db, const string &name, AttachInfo &info,
                                           AccessMode access_mode) {
	UCCredentials credentials;

	// check if we have a secret provided
	string secret_name;
	for (auto &entry : info.options) {
		auto lower_name = StringUtil::Lower(entry.first);
		if (lower_name == "type" || lower_name == "read_only") {
			// already handled
		} else if (lower_name == "secret") {
			secret_name = entry.second.ToString();
		} else {
			throw BinderException("Unrecognized option for PC attach: %s", entry.first);
		}
	}

	// if no secret is specified we default to the unnamed mysql secret, if it
	// exists
	bool explicit_secret = !secret_name.empty();
	if (!explicit_secret) {
		// look up settings from the default unnamed mysql secret if none is
		// provided
		secret_name = "__default_polaris";
	}

	string connection_string = info.path;
	auto secret_entry = GetSecret(context, secret_name);
	if (secret_entry) {
		// secret found - read data
		const auto &kv_secret = dynamic_cast<const KeyValueSecret &>(*secret_entry->secret);
		string new_connection_info;

		Value token_val = kv_secret.TryGetValue("token");
		if (token_val.IsNull()) {
			throw std::runtime_error("Token is blank");
		}
		credentials.token = token_val.ToString();

		Value endpoint_val = kv_secret.TryGetValue("endpoint");
		credentials.endpoint = endpoint_val.IsNull() ? "" : endpoint_val.ToString();
		StringUtil::RTrim(credentials.endpoint, "/");

		Value aws_region_val = kv_secret.TryGetValue("aws_region");
		credentials.aws_region = endpoint_val.IsNull() ? "" : aws_region_val.ToString();

	} else if (explicit_secret) {
		// secret not found and one was explicitly provided - throw an error
		throw BinderException("Secret with name \"%s\" not found", secret_name);
	}

	// TODO: Check catalog with name actually exists!

	return make_uniq<UCCatalog>(db, info.path, access_mode, credentials);
}

static unique_ptr<TransactionManager> CreateTransactionManager(StorageExtensionInfo *storage_info, AttachedDatabase &db,
                                                               Catalog &catalog) {
	auto &uc_catalog = catalog.Cast<UCCatalog>();
	return make_uniq<UCTransactionManager>(db, uc_catalog);
}

class UCCatalogStorageExtension : public StorageExtension {
public:
	UCCatalogStorageExtension() {
		attach = PolarisCatalogAttach;
		create_transaction_manager = CreateTransactionManager;
	}
};

static void LoadInternal(DatabaseInstance &instance) {
	UCAPI::InitializeCurl();

	SecretType secret_type;
	secret_type.name = "polaris";
	secret_type.deserializer = KeyValueSecret::Deserialize<KeyValueSecret>;
	secret_type.default_provider = "config";

	ExtensionUtil::RegisterSecretType(instance, secret_type);

	CreateSecretFunction secret_function = {"polaris", "config", CreatePolarisSecretFunction};
	SetPolarisSecretParameters(secret_function);
	ExtensionUtil::RegisterFunction(instance, secret_function);

	auto &config = DBConfig::GetConfig(instance);
	config.storage_extensions["polaris"] = make_uniq<UCCatalogStorageExtension>();
}

void PolarisExtension::Load(DuckDB &db) {
	LoadInternal(*db.instance);
}
std::string PolarisExtension::Name() {
	return "polaris";
}

std::string PolarisExtension::Version() const {
#ifdef EXT_VERSION_POLARIS
	return EXT_VERSION_POLARIS;
#else
	return "";
#endif
}

} // namespace duckdb

extern "C" {

DUCKDB_EXTENSION_API void polaris_init(duckdb::DatabaseInstance &db) {
	duckdb::DuckDB db_wrapper(db);
	db_wrapper.LoadExtension<duckdb::PolarisExtension>();
}

DUCKDB_EXTENSION_API const char *polaris_version() {
	return duckdb::DuckDB::LibraryVersion();
}
}

#ifndef DUCKDB_EXTENSION_MAIN
#error DUCKDB_EXTENSION_MAIN not defined
#endif
