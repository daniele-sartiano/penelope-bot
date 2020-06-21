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
    CassCollection* links = nullptr;

    std::string query = "INSERT INTO ";
    query.append(this->db);
    query.append(".");
    query.append(this->table);
    query.append(" (timestamp, link, text, links) VALUES (?, ?, ?, ?);");
    statement = cass_statement_new(query.c_str(), 4);

    cass_statement_bind_int64(statement, 0, model.getTimestamp());
    cass_statement_bind_string(statement, 1, model.getLink().c_str());
    cass_statement_bind_string(statement, 2, model.getText().c_str());

    links = cass_collection_new(CASS_COLLECTION_TYPE_SET, model.getLinks().size());
    for (std::string l: model.getLinks()) {
        cass_collection_append_string(links, l.c_str());
    }

    cass_statement_bind_collection(statement, 3, links);
    cass_collection_free(links);

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

Model* DataManager::select_model(Model& model) {
    std::string s(model.getLink());
    return this->select_model(s);
}

Model* DataManager::select_model(std::string& link) {
    CassError rc = CASS_OK;
    CassStatement* statement = NULL;
    CassFuture* future = NULL;
    std::string query = "SELECT * FROM ";
    query.append(this->db);
    query.append(".");
    query.append(this->table);
    query.append(" WHERE link = ?");

    statement = cass_statement_new(query.c_str(), 1);

    cass_statement_bind_string(statement, 0, link.c_str());

    future = cass_session_execute(this->session, statement);
    cass_future_wait(future);

    rc = cass_future_error_code(future);
    if (rc != CASS_OK) {
        log_error(future);
    } else {
        const CassResult* result = cass_future_get_result(future);
        CassIterator* iterator = cass_iterator_from_result(result);

        long qtimestamp;
        const char* qlink;
        size_t qlink_size;
        const char* qtext;
        size_t qtext_size;

        if (cass_iterator_next(iterator)) {
            const CassRow* row = cass_iterator_get_row(iterator);
            cass_value_get_int64(cass_row_get_column_by_name(row, "timestamp"), &qtimestamp);
            cass_value_get_string(cass_row_get_column_by_name(row, "link"), &qlink, &qlink_size);
            cass_value_get_string(cass_row_get_column_by_name(row, "text"), &qtext, &qtext_size);
            std::set<std::string> s;

            cass_result_free(result);
            cass_iterator_free(iterator);
            
            return new Model(qtimestamp, std::string(qlink), std::string(qtext), "", s);

        }

        cass_result_free(result);
        cass_iterator_free(iterator);
    }

    cass_future_free(future);
    cass_statement_free(statement);

    return nullptr;
}

DataManager::~DataManager() {
    cass_session_free(this->session);
    cass_cluster_free(this->cluster);
}
