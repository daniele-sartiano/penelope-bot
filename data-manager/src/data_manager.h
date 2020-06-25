//
// Created by lele on 21/05/20.
//

#ifndef DATA_MANAGER_DATA_MANAGER_H
#define DATA_MANAGER_DATA_MANAGER_H


#include <string>
#include <cassandra.h>
#include <model.h>

class DataManager {
private:
    const static int REPLICATION_FACTOR = 1;

    CassCluster* cluster;
    CassSession* session;
    std::string db = "penelopebot";
    std::string table = "data";

    CassError connect_session(CassSession* session, const CassCluster* cluster);
    static void log_error(CassFuture* future);
public:
    explicit DataManager(const std::string& hosts, const std::string& dbname = "", const std::string& tablename = "") {
        this->cluster = nullptr;
        this->session = cass_session_new();

        this->cluster = cass_cluster_new();
        cass_cluster_set_contact_points(cluster, hosts.c_str());

        if (connect_session(this->session, this->cluster) != CASS_OK) {
            cass_cluster_free(this->cluster);
            cass_session_free(this->session);
            exit(EXIT_FAILURE);
        }

        if (!dbname.empty()) {
            this->db = dbname;
        }

        std::string q = "CREATE KEYSPACE IF NOT EXISTS ";
        q.append(this->db);
        q.append(" WITH replication = { 'class': 'SimpleStrategy', 'replication_factor': '");
        q.append(std::to_string(REPLICATION_FACTOR));
        q.append("' };");
        this->query(q);

        if (!tablename.empty()) {
            this->table = tablename;
        }
        q = "CREATE TABLE IF NOT EXISTS ";
        q.append(this->db);
        q.append(".");
        q.append(this->table);
        q.append(" (timestamp bigint, link text, text text, ip inet, links set<text>, PRIMARY KEY (link));");

        this->query(q);
    }

    virtual ~DataManager();

    CassError query(const std::string& query);
    CassError insert_model(const Model& model);
    Model* select_model(Model& model);
    Model* select_model(std::string& link);
};


#endif //DATA_MANAGER_DATA_MANAGER_H
