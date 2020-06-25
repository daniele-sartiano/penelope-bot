#include <hiredis/hiredis.h>
#include "src/Downloader.h"

#include <natscommunication.h>

#include <getopt.h>
#include <iostream>

static void onMsg(natsConnection *nc, natsSubscription *sub, natsMsg *msg, void *closure) {
    /*printf("Received msg: %s - %.*s\n",
           natsMsg_GetSubject(msg),
           natsMsg_GetDataLength(msg),
           natsMsg_GetData(msg));*/

    std::string directory = getenv("DOWNLOAD_DIRECTORY") != nullptr ? getenv("DOWNLOAD_DIRECTORY") : "/tmp";
    std::string server = getenv("NATS_URI") != nullptr ? getenv("NATS_URI") : "nats://127.0.0.1:4222";
    std::string parser_subject = getenv("PARSER_SUBJECT") != nullptr ? getenv("PARSER_SUBJECT") : "parser";

    const clock_t begin_time = clock();
    Downloader d(natsMsg_GetData(msg));
    std::string r = d.download(directory);

    if (r.empty()) {
        std::cerr << "Discard " << msg << std::endl;
    } else {
        auto *producer = new NatsProducer(server);
        std::cout << "Sending msg" << std::endl;
        producer->send(parser_subject, r);
        delete producer;
    }

    // Need to destroy the message!
    natsMsg_Destroy(msg);
    std::cout << "Time: " << float( clock () - begin_time ) /  CLOCKS_PER_SEC << std::endl;
}

void* listen(void *thid) {

    std::cout << "Thread " << (long)thid << std::endl;

    std::string server = getenv("NATS_URI") != nullptr ? getenv("NATS_URI") : "nats://127.0.0.1:4222";
    std::string downloader_subject = getenv("DOWNLOADER_SUBJECT") != nullptr ? getenv("DOWNLOADER_SUBJECT") : "downloader";
    std::string downloader_queue = getenv("DOWNLOADER_QUEUE") != nullptr ? getenv("DOWNLOADER_QUEUE") : "qdownloader";
    auto *receiver = new NatsReceiver(server);
    receiver->subscribe(downloader_subject, downloader_queue, onMsg, NULL);
}

int main(int argc, char **argv) {
    int NUMT = 100;
    pthread_t tid[NUMT];
    int error;

    for (int i=0; i<NUMT; i++) {
        error = pthread_create(
                &tid[i],
                NULL,
                listen,
                (void *)i);
        if (0 != error) {
            std::cerr << "Could't run thread number " << i << " errno " << error << std::endl;
        }
    }

    for(int i=0; i< NUMT; i++) {
        error = pthread_join(tid[i], NULL);
        std::cout << "Thread " << i << " terminated" << std::endl;
    }

    pthread_exit(NULL);

    return 0;
}