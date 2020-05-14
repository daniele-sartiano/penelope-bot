//
// Created by lele on 31/07/19.
//

#ifndef DOWNLOADER_DOWNLOADER_H
#define DOWNLOADER_DOWNLOADER_H

#include <string>

class Downloader {
    const char *url;
    static size_t write_data(void *ptr, size_t size, size_t nmemb, void *stream);
    void download(std::string &directory);
public:
    Downloader(const char *url, std::string directory=nullptr) {
        this->url = url;
        download(directory);
    }
};


#endif //DOWNLOADER_DOWNLOADER_H
