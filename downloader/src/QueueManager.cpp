//
// Created by lele on 31/07/19.
//

#include <iostream>
#include <cstring>
#include "QueueManager.h"
#include "Downloader.h"

void RedisQueueManager::set(const char * key, const char *message) {
    redisReply *reply;
    /* Set a key */
    reply = static_cast<redisReply *>(redisCommand(this->context, "RPUSH %s %s", key, message));
    printf("SET: %s\n", reply->str);
    freeReplyObject(reply);
}

void RedisQueueManager::get(const char *key) {
    redisReply *reply;
    reply = static_cast<redisReply *>(redisCommand(this->context, "LPOP %s", key));
    printf("GET %s: %s\n", key, reply->str);

    Downloader(reply->str);

    freeReplyObject(reply);
}

// https://github.com/edenhill/librdkafka/blob/master/examples/producer.cpp
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

RdKafka::Consumer *KafkaQueueManager::get_consumer() {
    std::string errstr;
    if (!this->consumer) {
        this->consumer = RdKafka::Consumer::create(this->conf, errstr);
        if (!this->consumer) {
            std::cerr << "Failed to create consumer: " << errstr << std::endl;
            exit(1);
        }
    }
    std::cout << "% Created consumer " << consumer->name() << std::endl;
    return this->consumer;
}

void KafkaQueueManager::set(const char *key, const char *message) {
    std::string errstr;
    printf("Kafka SET\n");
    auto *producer = this->get_producer();
    RdKafka::ErrorCode err = producer->produce(
            std::string(key),
            RdKafka::Topic::PARTITION_UA,
            RdKafka::Producer::RK_MSG_COPY,
            (void *) message,
            std::strlen(message),
            NULL,
            0,
            0,
            NULL);
    if (err != RdKafka::ERR_NO_ERROR) {
        std::cerr << "Failed to produce" << key << std::endl;
    } else {
        std::cerr << "% Enqueued message (" << std::strlen(message) << " bytes) " << "for topic " << message << std::endl;
    }
    producer->poll(0);
}

void KafkaQueueManager::get(const char *key) {
    std::string errstr;
    printf("Kafka GET\n");
    auto *consumer = this->get_consumer();
    printf("Consumer ok\n");
    RdKafka::Topic *topic = RdKafka::Topic::create(consumer, std::string(key), this->conf, errstr);
    if (!topic) {
        std::cerr << "Failed to create topic: " << errstr << std::endl;
        exit(1);
    }
    printf("Consumer ok\n");
    RdKafka::ErrorCode resp = consumer->start(topic, RdKafka::Topic::PARTITION_UA, RdKafka::Topic::OFFSET_BEGINNING);
    if (resp != RdKafka::ERR_NO_ERROR) {
        std::cerr << "Failed to start consumer: " << RdKafka::err2str(resp) << std::endl;
        exit(1);
    }

    std::cerr << "Consumer created" << std::endl;
    
    while (true) {
        RdKafka::Message *msg = consumer->consume(topic, RdKafka::Topic::PARTITION_UA, 1000);
        std::cerr << "Consumed" << msg->payload() << std::endl;
        delete msg;
        consumer->poll(0);
    }
}