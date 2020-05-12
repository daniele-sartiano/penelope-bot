//
// Created by lele on 12/05/20.
//

#ifndef DOWNLOADER_NATSPRODUCER_H
#define DOWNLOADER_NATSPRODUCER_H

#include <nats/nats.h>
#include <string>
#include <vector>

class NatsProducer {
public:
    explicit NatsProducer(const std::string &server) {
        this->conn = nullptr;
        natsStatus s;
        // Creates a connection to the default NATS URL
        s = natsConnection_ConnectTo(&this->conn, server.c_str());
        if (s != NATS_OK)
        {
            nats_PrintLastErrorStack(stderr);
            exit(2);
        }
    }
    ~NatsProducer() {
        natsConnection_Destroy(this->conn);
    }
    void send(const std::string &subject, std::vector<std::string> messages);
private:
    natsConnection *conn;
};


#endif //DOWNLOADER_NATSPRODUCER_H
