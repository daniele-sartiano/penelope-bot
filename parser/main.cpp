#include <iostream>
#include "src/Parser.h"

#include <natscommunication.h>


static void onMsg(natsConnection *nc, natsSubscription *sub, natsMsg *msg, void *closure) {
    printf("Received msg: %s - %.*s\n",
           natsMsg_GetSubject(msg),
           natsMsg_GetDataLength(msg),
           natsMsg_GetData(msg));

    auto *p = new Parser(natsMsg_GetData(msg));
    auto v = p->parse();
    std::cout << v << std::endl;

    std::string server = getenv("NATS_URI") != nullptr ? getenv("NATS_URI") : "nats://127.0.0.1:4222";
    std::string data_manager_subject = getenv("DATA_MANAGER_SUBJECT") != nullptr ? getenv("DATA_MANAGER_SUBJECT") : "data-manager";
    auto *producer = new NatsProducer(server);
    producer->send(data_manager_subject, v);
    // Need to destroy the message!
    natsMsg_Destroy(msg);
}

int main() {

    std::string server = getenv("NATS_URI") != nullptr ? getenv("NATS_URI") : "nats://127.0.0.1:4222";
    auto *receiver = new NatsReceiver(server);

    std::string subject = getenv("PARSER_SUBJECT") != nullptr ? getenv("PARSER_SUBJECT") : "parser";
    std::string queue = getenv("PARSER_QUEUE") != nullptr ? getenv("PARSER_QUEUE") : "qparser";

    std::cout << "subject " << subject << std::endl;

    receiver->subscribe(subject, queue, onMsg, nullptr);

    return EXIT_SUCCESS;
}
