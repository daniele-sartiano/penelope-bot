#include <iostream>
#include <atomic>
#include "src/Parser.h"

#include <natscommunication.h>

std::atomic<int> counter(0);

static void onMsg(natsConnection *nc, natsSubscription *sub, natsMsg *msg, void *closure) {
    auto *producer = static_cast<NatsProducer*>(closure);

    const clock_t begin_time = clock();
    Parser p(natsMsg_GetData(msg));
    auto v = p.parse();

    std::string data_manager_subject = getenv("DATA_MANAGER_SUBJECT") != nullptr ? getenv("DATA_MANAGER_SUBJECT") : "data-manager";
    producer->send(data_manager_subject, v);
    counter++;

    if (counter % 50 == 0) {
        std::cout << "Received " << counter << " messages" << std::endl;
    }

    // Need to destroy the message!
    natsMsg_Destroy(msg);
    std::cout << "Time: " << float( clock () - begin_time ) /  CLOCKS_PER_SEC << std::endl;
}

int main() {

    std::string server = getenv("NATS_URI") != nullptr ? getenv("NATS_URI") : "nats://127.0.0.1:4222";
    std::string subject = getenv("PARSER_SUBJECT") != nullptr ? getenv("PARSER_SUBJECT") : "parser";
    std::string queue = getenv("PARSER_QUEUE") != nullptr ? getenv("PARSER_QUEUE") : "qparser";

    std::cout << "subject " << subject << std::endl;

    NatsProducer producer(server);
    NatsReceiver receiver(server);
    receiver.subscribe(subject, queue, onMsg, static_cast<void*>(&producer));

    return EXIT_SUCCESS;
}
