//
// Created by lele on 31/07/19.
//

#ifndef DOWNLOADER_DOWNLOADER_H
#define DOWNLOADER_DOWNLOADER_H

#include <string>
#include <tuple>
#include <model.h>

class Downloader {
private:
    const std::string USER_AGENT = "penelope-bot";
    Model *model;

    static size_t write_data(void *ptr, size_t size, size_t nmemb, void *stream);
    std::tuple<std::string, long, bool> download(std::string &directory);
public:
    Downloader(const std::string& serialized, std::string directory="") {
        this->model = new Model(serialized);
        auto o = download(directory);
    }

    bool discard();
};


#endif //DOWNLOADER_DOWNLOADER_H
