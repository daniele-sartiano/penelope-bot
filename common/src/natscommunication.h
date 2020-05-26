#ifndef COMMON_NATSCOMMUNICATION_H
#define COMMON_NATSCOMMUNICATION_H

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
    void send(const std::string &subject, std::string message);
private:
    natsConnection *conn;
};

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

    void subscribe(const std::string &subject, std::string &queue, void (*f)(natsConnection *, natsSubscription *, natsMsg *, void *),
                   void * closure);
private:
    natsConnection *conn;
};

#endif //COMMON_NATSCOMMUNICATION_H
