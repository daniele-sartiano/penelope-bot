//
// Created by lele on 10/05/20.
//

#include <sys/time.h>
#include "KafkaReader.h"

static int64_t now () {
    struct timeval tv{};
    gettimeofday(&tv, nullptr);
    return ((int64_t)tv.tv_sec * 1000) + (tv.tv_usec / 1000);
}

std::vector<RdKafka::Message *> KafkaReader::get() {
    std::vector<RdKafka::Message *> msgs;
    msgs.reserve(KafkaReader::BATCH_SIZE);

    int64_t end = now() + KafkaReader::BATCH_TIMEOUT;
    int64_t remaining_timeout = KafkaReader::BATCH_TIMEOUT;

    while (msgs.size() < KafkaReader::BATCH_SIZE) {
        RdKafka::Message *msg = consumer->consume(remaining_timeout);

        switch (msg->err()) {
            case RdKafka::ERR__TIMED_OUT:
                delete msg;
                return msgs;

            case RdKafka::ERR_NO_ERROR:
                msgs.push_back(msg);
                break;

            default:
                std::cerr << "%% Consumer error: " << msg->errstr() << std::endl;
                throw msg->errstr();
        }

        remaining_timeout = end - now();
        if (remaining_timeout < 0)
            break;
    }
    return msgs;
}
