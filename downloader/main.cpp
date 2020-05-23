#include <hiredis/hiredis.h>
#include "src/Downloader.h"

#include <natscommunication.h>

#include <getopt.h>
#include <iostream>

static void onMsg(natsConnection *nc, natsSubscription *sub, natsMsg *msg, void *closure) {
    printf("Received msg: %s - %.*s\n",
           natsMsg_GetSubject(msg),
           natsMsg_GetDataLength(msg),
           natsMsg_GetData(msg));

    std::string &directory = *(static_cast<std::string*>(closure));

    std::cout << "directory -> " << directory << std::endl;

    const clock_t begin_time = clock();
    Downloader(natsMsg_GetData(msg), directory);
    std::cout << "Time: " << float( clock () - begin_time ) /  CLOCKS_PER_SEC << std::endl;

    // Need to destroy the message!
    natsMsg_Destroy(msg);
}

int main(int argc, char **argv) {

    int opt;
    std::string directory;

    while((opt = getopt(argc, argv, "d:")) != -1) {
        switch(opt) {
            case 'd':
                printf("downloader directory: %s\n", optarg);
                directory = optarg;
                std::cout << directory << std::endl;
                break;
            case '?':
                printf("unknown option: %c\n", optopt);
                break;
        }
    }

    /*std::vector<std::string> urls;
    urls.emplace_back("www.sartiano.info");
    urls.emplace_back("www.repubblica.it");
    urls.emplace_back("www.corriere.it");

    auto *producer = new NatsProducer("nats://127.0.0.1:4222");

    for (int i=0; i<100000; i++) {
        producer->send("foo", urls);
    }*/

    std::string server = getenv("NATS_URI") != nullptr ? getenv("NATS_URI") : "nats://127.0.0.1:4222";
    auto *receiver = new NatsReceiver(server);

    std::string subject = getenv("SUBJECT") != nullptr ? getenv("SUBJECT") : "downloader";
    std::string queue = getenv("QUEUE") != nullptr ? getenv("QUEUE") : "qdownloader";

    std::cout << "subject " << subject << std::endl;

    receiver->subscribe(subject, queue, onMsg, static_cast<void*>(&directory));

    return 0;
}