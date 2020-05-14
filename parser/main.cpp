#include <iostream>
#include "src/Parser.h"

#include <natscommunication.h>


static void onMsg(natsConnection *nc, natsSubscription *sub, natsMsg *msg, void *closure) {
    printf("Received msg: %s - %.*s\n",
           natsMsg_GetSubject(msg),
           natsMsg_GetDataLength(msg),
           natsMsg_GetData(msg));

    std::string &directory = *(static_cast<std::string*>(closure));

    std::cout << "directory -> " << directory << std::endl;

    auto *p = new Parser();
    p->parse(natsMsg_GetData(msg));

    // Need to destroy the message!
    natsMsg_Destroy(msg);
}

int main() {
    std::cout << "Hello, World!" << std::endl;

    std::string server = getenv("NATS_URI") != nullptr ? getenv("NATS_URI") : "nats://127.0.0.1:4222";
    auto *receiver = new NatsReceiver(server);

    std::string subject = getenv("SUBJECT") != nullptr ? getenv("SUBJECT") : "parser";
    std::string queue = getenv("QUEUE") != nullptr ? getenv("QUEUE") : "qparser";

    std::cout << "subject " << subject << std::endl;

    receiver->subscribe(subject, queue, onMsg, nullptr);

    return EXIT_SUCCESS;
}
