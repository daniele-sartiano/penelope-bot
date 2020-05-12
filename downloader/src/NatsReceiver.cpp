//
// Created by Daniele Sartiano on 12/05/20.
//

#include "NatsReceiver.h"

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