#include <iostream>
#include <natscommunication.h>
#include <assert.h>
#include "src/data_manager.h"


static void onMsg(natsConnection *nc, natsSubscription *sub, natsMsg *msg, void *closure) {
    /*printf("Received msg: %s - %.*s\n",
           natsMsg_GetSubject(msg),
           natsMsg_GetDataLength(msg),
           natsMsg_GetData(msg));*/

    int counter = 0;
    auto d = static_cast<DataManager*>(closure);
    std::vector<Model> models = Model::deserialize_models(natsMsg_GetData(msg));
    std::vector<Model> out_models;
    const clock_t begin_time = clock();
    for (auto m: models) {
        //std::cout << "Received " << m.serialize() << std::endl;
        if (d->select_model(m) == nullptr) {
            d->insert_model(m);
        }

        for (std::string l : m.get_links()) {
            Model* selected = d->select_model(l);
            // if it does not exist in the db, then it sends the model to the downloader
            if (selected == nullptr) {
                std::set<std::string> s;
                Model m_out(0, l, "", "", "", s);
                out_models.push_back(m_out);
            } else {
                delete selected;
            }
        }
    }


    std::string downloader_subject = getenv("DOWNLOADER_SUBJECT") != nullptr ? getenv("DOWNLOADER_SUBJECT") : "downloader";
    std::string server = getenv("NATS_URI") != nullptr ? getenv("NATS_URI") : "nats://127.0.0.1:4222";
    auto *producer = new NatsProducer(server);

    std::vector<Model> subv;

    for (auto m: out_models) {
        subv.push_back(m);
        if (subv.size() == 5) {
            producer->send(downloader_subject, Model::serialize_models(subv));
            counter++;
            subv.clear();
            assert(subv.empty());
        }
    }

    if (!subv.empty()) {
        counter++;
        producer->send(downloader_subject, Model::serialize_models(subv));
        subv.clear();
    }

    // We need to destroy the message!
    natsMsg_Destroy(msg);
    delete producer;
    models.clear();
    out_models.clear();
    std::cout << "time: " << float( clock () - begin_time ) /  CLOCKS_PER_SEC << " - " << counter << " msg sent" << std::endl;
}

int main(int argc, char* argv[]) {

    if (argc > 2) {
        std::cout << "Not recognize the arguments" << std::endl;
        return 1;
    }

    std::string scylladb = getenv("SCYLLADB") != nullptr ? getenv("SCYLLADB") : "127.0.0.1";
    std::string server = getenv("NATS_URI") != nullptr ? getenv("NATS_URI") : "nats://127.0.0.1:4222";
    std::string subject = getenv("DATA_MANAGER_SUBJECT") != nullptr ? getenv("DATA_MANAGER_SUBJECT") : "data-manager";
    std::string queue = getenv("DATA_MANAGER_QUEUE") != nullptr ? getenv("DATA_MANAGER_QUEUE") : "qdata-manager";

    DataManager d(scylladb);

    // no arguments
    if (argc == 1) {
        auto *receiver = new NatsReceiver(server);
        receiver->subscribe(subject, queue, onMsg, static_cast<void*>(&d));
    } else if (argc == 2 && std::string(argv[1]) == "--boostrap") {
        std::cout << "TODO" << std::endl;
    }

    return 0;

}
