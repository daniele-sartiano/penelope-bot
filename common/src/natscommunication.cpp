#include "natscommunication.h"

#include <iostream>

void NatsProducer::send(const std::string &subject, std::string message) {
    natsStatus s = natsConnection_Publish(this->conn, subject.c_str(), message.c_str(), message.size());
    if (s != NATS_OK) {
        std::cerr << "message NOT sent" << std::endl;
        nats_PrintLastErrorStack(stderr);
    }
}

void NatsProducer::send(const std::string &subject, std::vector<std::string> messages) {
    for (std::string message : messages) {
        this->send(subject, message);
    }
}


void NatsReceiver::subscribe(const std::string &subject, std::string &queue, void (*f)(natsConnection *nc, natsSubscription *sub, natsMsg *msg, void *closure), void *clos) {
    natsStatus s;
    natsSubscription *sub  = nullptr;
    s = natsConnection_QueueSubscribe(&sub, this->conn, subject.c_str(), queue.c_str(), f, clos);
    if (s != NATS_OK) {
        nats_PrintLastErrorStack(stderr);
        exit(2);
    }
    for (;;) {
        nats_Sleep(100);
    }
}