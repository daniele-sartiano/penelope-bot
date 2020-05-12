//
// Created by lele on 31/07/19.
//

#ifndef DOWNLOADER_QUEUEMANAGER_H
#define DOWNLOADER_QUEUEMANAGER_H


#include <hiredis/hiredis.h>
#include <librdkafka/rdkafkacpp.h>
#include <csignal>


static volatile sig_atomic_t run = 1;
static bool exit_eof = false;
static int eof_cnt = 0;
static int partition_cnt = 0;
static int verbosity = 1;
static long msg_cnt = 0;
static int64_t msg_bytes = 0;
static void sigterm (int sig) {
    run = 0;
}


/**
 * @brief format a string timestamp from the current time
 */
static void print_time () {
#ifndef _MSC_VER
    struct timeval tv;
    char buf[64];
    gettimeofday(&tv, NULL);
    strftime(buf, sizeof(buf) - 1, "%Y-%m-%d %H:%M:%S", localtime(&tv.tv_sec));
    fprintf(stderr, "%s.%03d: ", buf, (int)(tv.tv_usec / 1000));
#else
    std::wcerr << CTime::GetCurrentTime().Format(_T("%Y-%m-%d %H:%M:%S")).GetString()
                << ": ";
#endif
}
class ExampleEventCb : public RdKafka::EventCb {
public:
    void event_cb (RdKafka::Event &event) {

        print_time();

        switch (event.type())
        {
            case RdKafka::Event::EVENT_ERROR:
                std::cerr << "ERROR (" << RdKafka::err2str(event.err()) << "): " <<
                          event.str() << std::endl;
                break;

            case RdKafka::Event::EVENT_STATS:
                std::cerr << "\"STATS\": " << event.str() << std::endl;
                break;

            case RdKafka::Event::EVENT_LOG:
                fprintf(stderr, "LOG-%i-%s: %s\n",
                        event.severity(), event.fac().c_str(), event.str().c_str());
                break;

            case RdKafka::Event::EVENT_THROTTLE:
                std::cerr << "THROTTLED: " << event.throttle_time() << "ms by " <<
                          event.broker_name() << " id " << (int)event.broker_id() << std::endl;
                break;

            default:
                std::cerr << "EVENT " << event.type() <<
                          " (" << RdKafka::err2str(event.err()) << "): " <<
                          event.str() << std::endl;
                break;
        }
    }
};


class ExampleRebalanceCb : public RdKafka::RebalanceCb {
private:
    static void part_list_print (const std::vector<RdKafka::TopicPartition*>&partitions){
        for (unsigned int i = 0 ; i < partitions.size() ; i++)
            std::cerr << partitions[i]->topic() <<
                      "[" << partitions[i]->partition() << "], ";
        std::cerr << "\n";
    }
public:
    void rebalance_cb (RdKafka::KafkaConsumer *consumer,
                       RdKafka::ErrorCode err,
                       std::vector<RdKafka::TopicPartition*> &partitions) {
        std::cerr << "RebalanceCb: " << RdKafka::err2str(err) << ": ";

        part_list_print(partitions);

        if (err == RdKafka::ERR__ASSIGN_PARTITIONS) {
            consumer->assign(partitions);
            partition_cnt = (int)partitions.size();
        } else {
            consumer->unassign();
            partition_cnt = 0;
        }
        eof_cnt = 0;
    }
};



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
    RdKafka::KafkaConsumer *consumer;
public:
    KafkaQueueManager(const char *brokers) {
        std::string errstr;
        conf = RdKafka::Conf::create(RdKafka::Conf::CONF_GLOBAL);

        ExampleRebalanceCb ex_rebalance_cb;
        conf->set("rebalance_cb", &ex_rebalance_cb, errstr);

        if (conf->set("group.id",  "domain2crawl", errstr) != RdKafka::Conf::CONF_OK) {
            std::cerr << errstr << std::endl;
            exit(1);
        }

        if (conf->set("enable.partition.eof", "true", errstr) != RdKafka::Conf::CONF_OK) {
            std::cerr << errstr << std::endl;
            exit(1);
        }

        /*if (conf->set("bootstrap.servers", brokers, errstr) != RdKafka::Conf::CONF_OK) {
            std::cerr << errstr << std::endl;
            exit(1);
        }*/

        if (conf->set("debug", "broker,topic", errstr) != RdKafka::Conf::CONF_OK) {
            std::cerr << errstr << std::endl;
            exit(1);
        }

        std::string features;
        conf->get("builtin.features", features);
        printf("Kafka builtin Features %s\n", features.c_str());


        conf->set("metadata.broker.list", brokers, errstr);
        //conf->set("enable.partition.eof", "true", errstr);

        /*ExampleEventCb ex_event_cb;
        conf->set("event_cb", &ex_event_cb, errstr);*/

        tconf = RdKafka::Conf::create(RdKafka::Conf::CONF_TOPIC);
        conf->set("default_topic_conf", tconf, errstr);
    }
    void set(const char *key, const char *message) override;
    void get(const char *key) override;

private:
    RdKafka::Producer *get_producer();
    RdKafka::KafkaConsumer *get_consumer();
};

#endif //DOWNLOADER_QUEUEMANAGER_H
