//
// Created by lele on 21/05/20.
//

#ifndef DATA_MANAGER_MODEL_H
#define DATA_MANAGER_MODEL_H


#include <string>
#include <utility>
#include <rapidjson/document.h>

using namespace rapidjson;

class Model {
private:
    int timestamp;
    std::string link;
    std::string text;

public:
    Model(int timestamp, std::string link, std::string text) : timestamp(timestamp), link(std::move(link)),
                                                                             text(std::move(text)) {};
    explicit Model(const std::string& serialized) {
        Document d;
        d.Parse(serialized.c_str());
        Value &v = d["timestamp"];
        this->timestamp = v.GetInt64();
        v = d["link"];
        this->link = v.GetString();
        v = d["text"];
        this->text = v.GetString();
    }

    int getTimestamp() const {
        return timestamp;
    }

    const std::string &getLink() const {
        return link;
    }

    const std::string &getText() const {
        return text;
    };

};


#endif //DATA_MANAGER_MODEL_H
