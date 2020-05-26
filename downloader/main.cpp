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

    std::string directory = getenv("DOWNLOAD_DIRECTORY") != nullptr ? getenv("DOWNLOAD_DIRECTORY") : "";

    std::string server = getenv("NATS_URI") != nullptr ? getenv("NATS_URI") : "nats://127.0.0.1:4222";

    const clock_t begin_time = clock();
    Downloader d(natsMsg_GetData(msg));
    std::string r = d.download(directory);
    std::cout << "Time: " << float( clock () - begin_time ) /  CLOCKS_PER_SEC << std::endl;
    if (r.empty()) {
        std::cerr << "Discard " << msg << std::endl;
    } else {
        std::string parser_subject = getenv("PARSER_SUBJECT") != nullptr ? getenv("PARSER_SUBJECT") : "parser";
        auto *producer = new NatsProducer(server);
        producer->send(parser_subject, r);
    }

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

    std::string downloader_subject = getenv("DOWNLOADER_SUBJECT") != nullptr ? getenv("DOWNLOADER_SUBJECT") : "downloader";
    std::string downloader_queue = getenv("DOWNLOADER_QUEUE") != nullptr ? getenv("DOWNLOADER_QUEUE") : "qdownloader";

    receiver->subscribe(downloader_subject, downloader_queue, onMsg, NULL);

    return 0;
}