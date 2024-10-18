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
    query.append(" (timestamp, link, text, ip, links) VALUES (?, ?, ?, ?, ?);");
    statement = cass_statement_new(query.c_str(), 5);

    cass_statement_bind_int64(statement, 0, model.get_timestamp());
    cass_statement_bind_string(statement, 1, model.get_link().c_str());
    cass_statement_bind_string(statement, 2, model.get_text().c_str());
    CassInet inet;

    cass_inet_from_string(model.get_ip().c_str(), &inet);
    cass_statement_bind_inet(statement, 3, inet);

    links = cass_collection_new(CASS_COLLECTION_TYPE_SET, model.get_links().size());
    for (std::string l: model.get_links()) {
        cass_collection_append_string(links, l.c_str());
    }

    cass_statement_bind_collection(statement, 4, links);
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
    std::string s(model.get_link());
    return this->select_model(s);
}

std::vector<Model> DataManager::select_models() {
    std::vector<Model> models;

    CassError rc;
    CassStatement* statement = nullptr;
    CassFuture* future = nullptr;
    CassCollection* links = nullptr;

    std::string query = "SELECT links FROM ";
    query.append(this->db);
    query.append(".");
    query.append(this->table);
    statement = cass_statement_new(query.c_str(), 0);

    if (cass_future_error_code(future) == CASS_OK) {
        /* Retrieve result set and get the first row */
        const CassResult* result = cass_future_get_result(future);
        CassIterator* iterator = cass_iterator_from_result(result);

        while (cass_iterator_next(iterator)) {
            const CassValue* value = nullptr;
            CassIterator* items_iterator = nullptr;
            const CassRow* row = cass_iterator_get_row(iterator);

            value = cass_row_get_column(row, 0);
            items_iterator = cass_iterator_from_collection(value);

            while (cass_iterator_next(items_iterator)) {
                const char* link;
                size_t link_length;
                cass_value_get_string(cass_iterator_get_value(items_iterator), &link, &link_length);
                printf("link: %.*s\n", (int)link_length, link);
                std::set<std::string> s;
                Model m_out(0, link, "", "", "", s);
                models.push_back(m_out);
            }
            cass_iterator_free(items_iterator);
        }

        cass_result_free(result);
        cass_iterator_free(iterator);
    } else {
        /* Handle error */
        const char* message;
        size_t message_length;
        cass_future_error_message(future, &message, &message_length);
        fprintf(stderr, "Unable to run query: '%.*s'\n", (int)message_length, message);
    }

    cass_future_free(future);
    cass_statement_free(statement);

}

Model* DataManager::select_model(std::string& link) {
    CassError rc = CASS_OK;
    CassStatement* statement = NULL;
    CassFuture* future = NULL;
    std::string query = "SELECT timestamp, link, text FROM ";
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
            cass_future_free(future);
            cass_statement_free(statement);

            return new Model(qtimestamp, std::string(qlink), std::string(qtext), "", "", s);

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
