//
// Created by lele on 31/07/19.
//

#ifndef DOWNLOADER_QUEUEMANAGER_H
#define DOWNLOADER_QUEUEMANAGER_H


#include <hiredis/hiredis.h>
#include <librdkafka/rdkafkacpp.h>

class QueueManager {
    virtual void set(const char *key, const char *message) = 0;
    virtual void get(const char *key) = 0;

};

class RedisQueueManager : public QueueManager {
    redisContext *context;
public:
    RedisQueueManager(int port, const char *hostname, int sec, int usec) {
        struct timeval timeout = { sec, usec }; // 1.5 seconds
        context = redisConnectWithTimeout(hostname, port, timeout);
    }
    void set(const char *key, const char *message) override;
    void get(const char *key) override;

};

// https://docs.confluent.io/current/clients/c_cpp.html
class KafkaQueueManager : public QueueManager {
    RdKafka::Conf *conf;
    RdKafka::Conf *tconf;
    RdKafka::Producer *producer;
    RdKafka::Consumer *consumer;
public:
    KafkaQueueManager(const char *brokers) {
        std::string errstr;
        conf = RdKafka::Conf::create(RdKafka::Conf::CONF_GLOBAL);
        tconf = RdKafka::Conf::create(RdKafka::Conf::CONF_TOPIC);

        std::string features;
        conf->get("builtin.features", features);
        printf("Kafka builtin Features %s\n", features.c_str());

        conf->set("metadata.broker.list", brokers, errstr);
    }
    void set(const char *key, const char *message) override;
    void get(const char *key) override;

private:
    RdKafka::Producer *get_producer();
    RdKafka::Consumer *get_consumer();
};

#endif //DOWNLOADER_QUEUEMANAGER_H
