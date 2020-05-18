#include <iostream>
#include <cassandra.h>

struct Basic_ {
    const char *url;
    const char *text;
};

typedef struct Basic_ Basic;

void print_error(CassFuture* future) {
    const char* message;
    size_t message_length;
    cass_future_error_message(future, &message, &message_length);
    fprintf(stderr, "Error: %.*s\n", (int)message_length, message);
}

CassError connect_session(CassSession* session, const CassCluster* cluster) {
    CassError rc = CASS_OK;
    CassFuture* future = cass_session_connect(session, cluster);

    cass_future_wait(future);
    rc = cass_future_error_code(future);
    if (rc != CASS_OK) {
        print_error(future);
    }
    cass_future_free(future);

    return rc;
}


CassError execute_query(CassSession* session, const char* query) {
    CassError rc = CASS_OK;
    CassFuture* future = NULL;
    CassStatement* statement = cass_statement_new(query, 0);

    future = cass_session_execute(session, statement);
    cass_future_wait(future);

    rc = cass_future_error_code(future);
    if (rc != CASS_OK) {
        print_error(future);
    }

    cass_future_free(future);
    cass_statement_free(statement);

    return rc;
}

CassError insert_into_basic(CassSession* session, const Basic* basic) {
    CassError rc = CASS_OK;
    CassStatement* statement = NULL;
    CassFuture* future = NULL;
    const char* query =
            "INSERT INTO examples.basic (url, text) VALUES (?, ?);";

    statement = cass_statement_new(query, 2);

    cass_statement_bind_string(statement, 0, basic->url);
    cass_statement_bind_string(statement, 1, basic->text);

    future = cass_session_execute(session, statement);
    cass_future_wait(future);

    rc = cass_future_error_code(future);
    if (rc != CASS_OK) {
        print_error(future);
    }

    cass_future_free(future);
    cass_statement_free(statement);

    return rc;
}


CassError select_from_basic(CassSession* session, const char* url, Basic* basic) {
    CassError rc = CASS_OK;
    CassStatement* statement = NULL;
    CassFuture* future = NULL;
    const char* query = "SELECT * FROM examples.basic WHERE url = ?";

    statement = cass_statement_new(query, 1);

    cass_statement_bind_string(statement, 0, url);

    future = cass_session_execute(session, statement);
    cass_future_wait(future);

    rc = cass_future_error_code(future);
    if (rc != CASS_OK) {
        print_error(future);
    } else {
        const CassResult* result = cass_future_get_result(future);
        CassIterator* iterator = cass_iterator_from_result(result);

        if (cass_iterator_next(iterator)) {
            const CassRow* row = cass_iterator_get_row(iterator);
            size_t s;
            cass_value_get_string(cass_row_get_column(row, 1), &basic->text, &s);
        }

        cass_result_free(result);
        cass_iterator_free(iterator);
    }

    cass_future_free(future);
    cass_statement_free(statement);

    return rc;
}

int main() {
    std::cout << "Hello, World!" << std::endl;

    CassCluster* cluster = NULL;
    CassSession* session = cass_session_new();
    char* hosts = "127.0.0.1";

    Basic input = { "www.example.it", "questo è un testo."};
    Basic output;

    cluster = cass_cluster_new();
    cass_cluster_set_contact_points(cluster, hosts);

    if (connect_session(session, cluster) != CASS_OK) {
        cass_cluster_free(cluster);
        cass_session_free(session);
        return EXIT_FAILURE;
    }


    execute_query(session, "CREATE KEYSPACE IF NOT EXISTS examples WITH replication = { 'class': 'SimpleStrategy', 'replication_factor': '3' };");

    execute_query(session, "CREATE TABLE IF NOT EXISTS examples.basic (url text, text text, PRIMARY KEY (url));");

    insert_into_basic(session, &input);

    select_from_basic(session, input.url, &output);

    return 0;
}
