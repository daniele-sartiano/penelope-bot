//
// Created by lele on 31/07/19.
//

#ifndef DOWNLOADER_DOWNLOADER_H
#define DOWNLOADER_DOWNLOADER_H


class Downloader {
    const char *url;
    static size_t write_data(void *ptr, size_t size, size_t nmemb, void *stream);
    void download();
public:
    Downloader(const char *url) {
        this->url = url;
        download();
    }
};


#endif //DOWNLOADER_DOWNLOADER_H
