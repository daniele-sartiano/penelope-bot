//
// Created by lele on 12/05/20.
//

#ifndef DOWNLOADER_NATSRECEIVER_H
#define DOWNLOADER_NATSRECEIVER_H

#include <string>
#include <nats/nats.h>

class NatsReceiver {
public:
    NatsReceiver(const std::string &server) {
        this->conn = NULL;
        natsStatus s;
        s = natsConnection_ConnectTo(&this->conn, server.c_str());
        if (s != NATS_OK)
        {
            nats_PrintLastErrorStack(stderr);
            exit(2);
        }
    }

    void subscribe(const std::string &subject, void (*f)(natsConnection *nc, natsSubscription *sub, natsMsg *msg, void *closure));
private:
    natsConnection *conn;
};


#endif //DOWNLOADER_NATSRECEIVER_H
