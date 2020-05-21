#include <iostream>
#include <natscommunication.h>
#include "src/DataManager.h"


static void onMsg(natsConnection *nc, natsSubscription *sub, natsMsg *msg, void *closure) {
    printf("Received msg: %s - %.*s\n",
           natsMsg_GetSubject(msg),
           natsMsg_GetDataLength(msg),
           natsMsg_GetData(msg));

    auto d = *(static_cast<DataManager*>(closure));
    const Model m(natsMsg_GetData(msg));
    d.insert_model(m);
    // Need to destroy the message!
    natsMsg_Destroy(msg);
}

int main() {
    std::cout << "Hello, World!" << std::endl;

/*

    const Model m(R"({"timestamp": 1590067123, "link": "www.unipi.it", "text": "prova prova prova due"})");
    d->insert_model(m);
*/
    DataManager d("127.0.0.1");
    std::string server = getenv("NATS_URI") != nullptr ? getenv("NATS_URI") : "nats://127.0.0.1:4222";
    auto *receiver = new NatsReceiver(server);

    std::string subject = getenv("SUBJECT") != nullptr ? getenv("SUBJECT") : "data-manager";
    std::string queue = getenv("QUEUE") != nullptr ? getenv("QUEUE") : "qdata-manager";

    std::cout << "subject " << subject << std::endl;

    receiver->subscribe(subject, queue, onMsg, static_cast<void*>(&d));

    return 0;
}
