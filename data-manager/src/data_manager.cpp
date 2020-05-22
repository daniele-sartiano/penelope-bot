//
// Created by lele on 21/05/20.
//

#include <iostream>
#include "data_manager.h"

void DataManager::log_error(CassFuture *future) {
    const char* message;
    size_t message_length;
    cass_future_error_message(future, &message, &message_length);
    std::cerr << message << std::endl;
}

CassError DataManager::connect_session(CassSession *session, const CassCluster *cluster) {
    CassError rc;
    CassFuture* future = cass_session_connect(session, cluster);

    cass_future_wait(future);
    rc = cass_future_error_code(future);
    if (rc != CASS_OK) {
        this->log_error(future);
    }
    cass_future_free(future);

    return rc;
}

CassError DataManager::query(const std::string& query) {
    CassError rc;
    CassFuture* future = nullptr;
    CassStatement* statement = cass_statement_new(query.c_str(), 0);

    future = cass_session_execute(session, statement);
    cass_future_wait(future);

    rc = cass_future_error_code(future);
    if (rc != CASS_OK) {
        log_error(future);
    }

    cass_future_free(future);
    cass_statement_free(statement);

    return rc;
}

CassError DataManager::insert_model(const Model& model) {
    CassError rc;
    CassStatement* statement = nullptr;
    CassFuture* future = nullptr;
    std::string query = "INSERT INTO ";
    query.append(this->db);
    query.append(".");
    query.append(this->table);
    query.append(" (timestamp, link, text) VALUES (?, ?, ?);");
    statement = cass_statement_new(query.c_str(), 3);

    cass_statement_bind_int64(statement, 0, model.getTimestamp());
    cass_statement_bind_string(statement, 1, model.getLink().c_str());
    cass_statement_bind_string(statement, 2, model.getText().c_str());

    future = cass_session_execute(this->session, statement);
    cass_future_wait(future);

    rc = cass_future_error_code(future);
    if (rc != CASS_OK) {
        log_error(future);
    }

    cass_future_free(future);
    cass_statement_free(statement);

    return rc;
}
