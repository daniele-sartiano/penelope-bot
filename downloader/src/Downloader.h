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
    size_t max_concurrent;
    std::vector<Model> models;

public:
    static const size_t DEFAULT_MAX_CONCURRENT = 20;

    explicit Downloader(const std::string& serialized, size_t max_concurrent = DEFAULT_MAX_CONCURRENT)
        : max_concurrent(max_concurrent) {
        this->models = Model::deserialize_models(serialized);
    }

    std::string download(const std::string &directory);

    std::vector<Model> get_models() const;
};


#endif //DOWNLOADER_DOWNLOADER_H
