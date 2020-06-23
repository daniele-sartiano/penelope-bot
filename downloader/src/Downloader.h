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
    std::vector<Model> models;

    static size_t write_data(void *ptr, size_t size, size_t nmemb, void *stream);
public:
    Downloader(const std::string& serialized) {
        this->models = Model::deserialize_models(serialized);
    }

    std::string download(std::string &directory);

    std::vector<Model> get_models() const;

    bool discard(Model& model);
};


#endif //DOWNLOADER_DOWNLOADER_H
