#include <iostream>
#include <natscommunication.h>
#include "src/data_manager.h"


static void onMsg(natsConnection *nc, natsSubscription *sub, natsMsg *msg, void *closure) {
    /*printf("Received msg: %s - %.*s\n",
           natsMsg_GetSubject(msg),
           natsMsg_GetDataLength(msg),
           natsMsg_GetData(msg));*/

    auto d = static_cast<DataManager*>(closure);

    Model m(natsMsg_GetData(msg));

    //std::cout << "Received " << m.serialize() << std::endl;

    const clock_t begin_time = clock();
    if (d->select_model(m) == nullptr) {
        d->insert_model(m);
    }

    std::string downloader_subject = getenv("DOWNLOADER_SUBJECT") != nullptr ? getenv("DOWNLOADER_SUBJECT") : "downloader";
    std::string server = getenv("NATS_URI") != nullptr ? getenv("NATS_URI") : "nats://127.0.0.1:4222";
    auto *producer = new NatsProducer(server);
    for (std::string l : m.getLinks()) {
        Model* selected = d->select_model(l);
        if (selected == nullptr) {
            std::set<std::string> s;
            Model m(0, l, "", "", s);
            //std::cout << "Sending " << l << std::endl;
            producer->send(downloader_subject, m.serialize());
        } else {
            delete selected;
        }
    }

    // Need to destroy the message!
    natsMsg_Destroy(msg);
    delete producer;
    std::cout << m.getLink() << " - time: " << float( clock () - begin_time ) /  CLOCKS_PER_SEC << std::endl;
}

int main() {

/*

    const Model m(R"({"timestamp": 1590067123, "link": "www.unipi.it", "text": "prova prova prova due"})");
    d->insert_model(m);
*/
    std::string scylladb = getenv("SCYLLADB") != nullptr ? getenv("SCYLLADB") : "127.0.0.1";
    DataManager d(scylladb);
    std::string server = getenv("NATS_URI") != nullptr ? getenv("NATS_URI") : "nats://127.0.0.1:4222";
    auto *receiver = new NatsReceiver(server);

    std::string subject = getenv("DATA_MANAGER_SUBJECT") != nullptr ? getenv("DATA_MANAGER_SUBJECT") : "data-manager";
    std::string queue = getenv("DATA_MANAGER_QUEUE") != nullptr ? getenv("DATA_MANAGER_QUEUE") : "qdata-manager";

    std::cout << "subject " << subject << std::endl;

    receiver->subscribe(subject, queue, onMsg, static_cast<void*>(&d));
    return 0;
}
