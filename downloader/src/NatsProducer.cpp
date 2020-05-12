//
// Created by lele on 12/05/20.
//

#include <iostream>
#include "NatsProducer.h"

void NatsProducer::send(const std::string &subject, std::vector<std::string> messages) {
    natsStatus s;
    for (std::string message : messages) {
        s = natsConnection_Publish(this->conn, subject.c_str(), message.c_str(), message.size());
        if (s == NATS_OK) {
            std::cout << "message sent" << std::endl;
        } else {
            std::cerr << "message NOT sent" << std::endl;
            nats_PrintLastErrorStack(stderr);
        }
    }
}