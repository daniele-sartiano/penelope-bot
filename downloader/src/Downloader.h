//
// Created by lele on 31/07/19.
//

#ifndef DOWNLOADER_DOWNLOADER_H
#define DOWNLOADER_DOWNLOADER_H

#include <string>
#include <rapidjson/document.h>

using namespace rapidjson;

class Downloader {
private:
    const std::string USER_AGENT = "penelope-bot";
    std::string link;
    long last_seen;

    static size_t write_data(void *ptr, size_t size, size_t nmemb, void *stream);
    std::tuple<std::string, long, bool> download(std::string &directory);
public:
    Downloader(const std::string& serialized, std::string directory=nullptr) {
        Document d;
        d.Parse(serialized.c_str());
        Value &v = d["last_seen"];
        this->last_seen = v.GetInt64();
        v = d["link"];
        this->link = v.GetString();
        download(directory);
    }

    bool discard();
};


#endif //DOWNLOADER_DOWNLOADER_H
