//
// Created by lele on 31/07/19.
//

#ifndef DOWNLOADER_QUEUEMANAGER_H
#define DOWNLOADER_QUEUEMANAGER_H


#include <hiredis/hiredis.h>

class QueueManager {
    redisContext *context;
public:
    QueueManager(int port, const char *hostname, int sec, int usec) {
        struct timeval timeout = { sec, usec }; // 1.5 seconds
        context = redisConnectWithTimeout(hostname, port, timeout);
    }
    void set(const char *key, const char *message);

    void get(const char *key);

};


#endif //DOWNLOADER_QUEUEMANAGER_H
