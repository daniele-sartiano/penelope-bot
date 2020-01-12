//
// Created by lele on 31/07/19.
//

#include <iostream>
#include "QueueManager.h"
#include "Downloader.h"

void RedisQueueManager::set(const char * key, const char *message) const {
    redisReply *reply;
    /* Set a key */
    reply = static_cast<redisReply *>(redisCommand(this->context, "RPUSH %s %s", key, message));
    printf("SET: %s\n", reply->str);
    freeReplyObject(reply);
}

void RedisQueueManager::get(const char *key) const {
    redisReply *reply;
    reply = static_cast<redisReply *>(redisCommand(this->context, "LPOP %s", key));
    printf("GET %s: %s\n", key, reply->str);

    Downloader(reply->str);

    freeReplyObject(reply);
}

RdKafka::Producer * KafkaQueueManager::get_producer() {
    std::string errstr;
    if (!this->producer) {
        this->producer = RdKafka::Producer::create(this->conf, errstr);
        if (!this->producer) {
            std::cerr << "Failed to create producer: " << errstr << std::endl;
            exit(1);
        }
        std::cout << "% Created producer " << producer->name() << std::endl;
    }
    return this->producer;
}

void KafkaQueueManager::set(const char *key, const char *message) {
    std::string errstr;
    printf("Kafka SET\n");
    this->get_producer()

}

void KafkaQueueManager::get(const char *key) const {
    printf("Kafka GET\n");
}