//
// Created by lele on 10/05/20.
//

#ifndef DOWNLOADER_KAFKAREADER_H
#define DOWNLOADER_KAFKAREADER_H


#include <librdkafka/rdkafkacpp.h>
#include <iostream>

class KafkaReader {
public:
    KafkaReader(std::string servers, std::string groupid, std::vector<std::string> topics) {
        std::string errstr;
        RdKafka::Conf *conf = RdKafka::Conf::create(RdKafka::Conf::CONF_GLOBAL);
        if (conf->set("enable.partition.eof", "false", errstr) != RdKafka::Conf::CONF_OK) {
            std::cerr << errstr << std::endl;
            throw errstr;
        }

        if (conf->set("bootstrap.servers", servers, errstr) != RdKafka::Conf::CONF_OK) {
            std::cerr << errstr << std::endl;
            throw errstr;
        }

        if (conf->set("group.id",  groupid, errstr) != RdKafka::Conf::CONF_OK) {
            std::cerr << errstr << std::endl;
            throw errstr;
        }

        if (conf->set("auto.offset.reset",  "earliest", errstr) != RdKafka::Conf::CONF_OK) {
            std::cerr << errstr << std::endl;
            throw errstr;
        }

        this->consumer = RdKafka::KafkaConsumer::create(conf, errstr);

        if (!this->consumer) {
            std::cerr << "Failed to create consumer: " << errstr << std::endl;
            throw errstr;
        }

        delete conf;
        RdKafka::ErrorCode err = this->consumer->subscribe(topics);
        if (err) {
            std::cerr << "Failed to subscribe to " << topics.size() << " topics: "
                      << RdKafka::err2str(err) << std::endl;
            throw err;
        }
    }

    std::vector<RdKafka::Message *> get();

private:
    RdKafka::KafkaConsumer *consumer;
    static const size_t BATCH_SIZE = 100;
    static const int BATCH_TIMEOUT = 1000;
};


#endif //DOWNLOADER_KAFKAREADER_H
