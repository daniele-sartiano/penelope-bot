#include <iostream>
#include "src/Parser.h"

#include <natscommunication.h>


static void onMsg(natsConnection *nc, natsSubscription *sub, natsMsg *msg, void *closure) {

    const clock_t begin_time = clock();
    auto *p = new Parser(natsMsg_GetData(msg));
    auto v = p->parse();


    std::string server = getenv("NATS_URI") != nullptr ? getenv("NATS_URI") : "nats://127.0.0.1:4222";
    std::string data_manager_subject = getenv("DATA_MANAGER_SUBJECT") != nullptr ? getenv("DATA_MANAGER_SUBJECT") : "data-manager";
    auto *producer = new NatsProducer(server);
    producer->send(data_manager_subject, v);
    // Need to destroy the message!
    natsMsg_Destroy(msg);
    std::cout << p->get_model()->getLink() << " - time: " << float( clock () - begin_time ) /  CLOCKS_PER_SEC << std::endl;
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
