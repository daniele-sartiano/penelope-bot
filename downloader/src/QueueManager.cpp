//
// Created by lele on 31/07/19.
//

#include "QueueManager.h"
#include "Downloader.h"

void QueueManager::set(const char * key, const char *message) {
    redisReply *reply;
    /* Set a key */
    reply = static_cast<redisReply *>(redisCommand(this->context, "RPUSH %s %s", key, message));
    printf("SET: %s\n", reply->str);
    freeReplyObject(reply);
}

void QueueManager::get(const char *key) {
    redisReply *reply;
    reply = static_cast<redisReply *>(redisCommand(this->context, "LPOP %s", key));
    printf("GET %s: %s\n", key, reply->str);

    Downloader(reply->str);

    freeReplyObject(reply);

}
