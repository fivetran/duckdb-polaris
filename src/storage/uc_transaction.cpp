#include "storage/uc_transaction.hpp"
#include "storage/uc_catalog.hpp"
#include "duckdb/parser/parsed_data/create_view_info.hpp"
#include "duckdb/catalog/catalog_entry/index_catalog_entry.hpp"
#include "duckdb/catalog/catalog_entry/view_catalog_entry.hpp"

namespace duckdb {

UCTransaction::UCTransaction(UCCatalog &uc_catalog, TransactionManager &manager, ClientContext &context)
    : Transaction(manager, context), access_mode(uc_catalog.access_mode) {
	//	connection = UCConnection::Open(uc_catalog.path);
}

UCTransaction::~UCTransaction() = default;

void UCTransaction::Start() {
	transaction_state = UCTransactionState::TRANSACTION_NOT_YET_STARTED;
}
void UCTransaction::Commit() {
	if (transaction_state == UCTransactionState::TRANSACTION_STARTED) {
		transaction_state = UCTransactionState::TRANSACTION_FINISHED;
		//		connection.Execute("COMMIT");
	}
}
void UCTransaction::Rollback() {
	if (transaction_state == UCTransactionState::TRANSACTION_STARTED) {
		transaction_state = UCTransactionState::TRANSACTION_FINISHED;
		//		connection.Execute("ROLLBACK");
	}
}

// UCConnection &UCTransaction::GetConnection() {
//	if (transaction_state == UCTransactionState::TRANSACTION_NOT_YET_STARTED) {
//		transaction_state = UCTransactionState::TRANSACTION_STARTED;
//		string query = "START TRANSACTION";
//		if (access_mode == AccessMode::READ_ONLY) {
//			query += " READ ONLY";
//		}
////		conne/**/ction.Execute(query);
//	}
//	return connection;
//}

// unique_ptr<UCResult> UCTransaction::Query(const string &query) {
//	if (transaction_state == UCTransactionState::TRANSACTION_NOT_YET_STARTED) {
//		transaction_state = UCTransactionState::TRANSACTION_STARTED;
//		string transaction_start = "START TRANSACTION";
//		if (access_mode == AccessMode::READ_ONLY) {
//			transaction_start += " READ ONLY";
//		}
//		connection.Query(transaction_start);
//		return connection.Query(query);
//	}
//	return connection.Query(query);
//}

UCTransaction &UCTransaction::Get(ClientContext &context, Catalog &catalog) {
	return Transaction::Get(context, catalog).Cast<UCTransaction>();
}

} // namespace duckdb
