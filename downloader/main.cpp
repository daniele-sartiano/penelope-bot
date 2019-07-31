#include <iostream>
#include <fstream>
#include <pthread.h>
#include <curl/curl.h>

#include <hiredis/hiredis.h>
#include "src/QueueManager.h"

/*// https://stackoverflow.com/questions/9786150/save-curl-content-result-into-a-string-in-c
static size_t write_data_to_string(void *contents, size_t size, size_t nmemb, void *userp)
{
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

static size_t write_data(void *ptr, size_t size, size_t nmemb, void *stream)
{
    std::ofstream& outstream = *reinterpret_cast<std::ofstream*>(stream);
    outstream.write((const char*)ptr, size * nmemb);

    //size_t written = fwrite(ptr, size, nmemb, (FILE *)stream);
    return size*nmemb;
}*/


/*static void *pull_one_url(void *url)
{
    CURL *curl;

    std::string file_name = "page.";
    file_name.append((char*)url);
    file_name.append(".out");

    std::ofstream out_file;
    out_file.open(file_name);

    curl = curl_easy_init();

    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &out_file);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, true);

    curl_easy_perform(curl); *//* ignores error *//*

    out_file.close();

    curl_easy_cleanup(curl);

    return nullptr;
}*/


static void *get_url() {
    const char *hostname = "127.0.0.1";
    int port = 6379;
    auto *q = new QueueManager(port, hostname, 1, 500000);
    q->get("domain2crawl");
}

/*void test_redis(redisContext *c) {

    redisReply *reply;
    *//* PING server *//*
    reply = static_cast<redisReply *>(redisCommand(c, "PING"));
    printf("PING: %s\n", reply->str);
    freeReplyObject(reply);


    *//* Set a key *//*
    reply = static_cast<redisReply *>(redisCommand(c, "SET %s %s", "foo", "hello world"));
    printf("SET: %s\n", reply->str);
    freeReplyObject(reply);

    *//* Try a GET and two INCR *//*
    reply = static_cast<redisReply *>(redisCommand(c, "GET foo"));
    printf("GET foo: %s\n", reply->str);
    freeReplyObject(reply);
}*/


void test_init_queue() {
    const char *hostname = "127.0.0.1";
    int port = 6379;
    auto *q = new QueueManager(port, hostname, 1, 500000);

    const char * const urls[] = {
            "www.sartiano.info",
            "www.repubblica.it",
            "www.corriere.it"
    };

    for (const char *url: urls) {
        printf("---> %s\n", url);
        q->set("domain2crawl", url);
    }
}

int main(int argc, char **argv) {


    test_init_queue();


    int NUMT = 3;
    pthread_t tid[NUMT];
    int error;

    /*const char * const urls[] = {
            "www.sartiano.info",
            "www.repubblica.it",
            "www.corriere.it"
    };*/


/*
    redisContext *c;

    const char *hostname = "127.0.0.1";
    int port = 6379;

    struct timeval timeout = { 1, 500000 }; // 1.5 seconds
    c = redisConnectWithTimeout(hostname, port, timeout);

    if (c == NULL || c->err) {
        if (c) {
            printf("Connection error: %s\n", c->errstr);
            redisFree(c);
        } else {
            printf("Connection error: can't allocate redis context\n");
        }
        exit(1);
    }

    test_redis(c);
*/

    curl_global_init(CURL_GLOBAL_ALL);

/*
    for (int i=0; i<NUMT; i++) {
        error = pthread_create(
                &tid[i],
                NULL,
                pull_one_url,
                (void*)urls[i]);
        if (0 != error) {
            fprintf(stderr, "Could't run thread number %d, errno %d\n", i, error);
        } else {
            fprintf(stderr,  "Thread %d,  gets %s\n", i, urls[i]);
        }
    }
*/

    for (int i=0; i<NUMT; i++) {
        error = pthread_create(
                &tid[i],
                nullptr,
                reinterpret_cast<void *(*)(void *)>(get_url),
                nullptr);
        if (0 != error) {
            fprintf(stderr, "Could't run thread number %d, errno %d\n", i, error);
        } else {
            //fprintf(stderr,  "Thread %d,  gets %s\n", i, urls[i]);
            fprintf(stderr,  "Thread %d started\n", i);
        }
    }

    /* now wait for all threads to terminate */
    for(int i=0; i< NUMT; i++) {
        error = pthread_join(tid[i], nullptr);
        fprintf(stderr, "Thread %d terminated\n", i);
    }

    return 0;
}