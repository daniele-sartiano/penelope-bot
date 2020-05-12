//
// Created by Daniele Sartiano on 12/05/20.
//

#include "NatsReceiver.h"

void NatsReceiver::subscribe(const std::string &subject, void (*f)(natsConnection *nc, natsSubscription *sub, natsMsg *msg, void *closure)) {
    natsStatus s;
    natsSubscription *sub  = nullptr;
    s = natsConnection_Subscribe(&sub, this->conn, subject.c_str(), f, nullptr);
    if (s != NATS_OK) {
        nats_PrintLastErrorStack(stderr);
        exit(2);
    }
    for (;;) {
        nats_Sleep(100);
    }
}