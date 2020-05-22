//
// Created by lele on 22/05/20.
//

#ifndef COMMON_MODEL_H
#define COMMON_MODEL_H

#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <string>
#include <set>
#include <utility>

using namespace rapidjson;

class Model {
private:

    static constexpr const char *KEY_TIMESTAMP = "timestamp";
    static constexpr const char *KEY_LINK = "link";
    static constexpr const char *KEY_TEXT = "text";
    static constexpr const char *KEY_FILENAME = "filename";
    static constexpr const char *KEY_LINKS = "links";

    int timestamp = 0;
    std::string link = "";
    std::string text = "";
    std::string filename = "";
    std::set<std::string> links;

public:

    Model(int timestamp, std::string link, std::string text, std::string filename,
          std::set<std::string> links) : timestamp(timestamp), link(std::move(link)), text(std::move(text)),
                                         filename(std::move(filename)),
                                         links(std::move(links)) {}

    explicit Model(const std::string &serialized) {
        Document d;
        Value v;
        d.Parse(serialized.c_str());

        for (std::string k: {Model::KEY_TIMESTAMP, Model::KEY_LINK, Model::KEY_TEXT, Model::KEY_FILENAME, Model::KEY_LINKS}) {
            if (d.HasMember(k.c_str())) {
                v = d[k.c_str()];
                if (k == Model::KEY_TIMESTAMP) {
                    this->timestamp = v.GetInt64();
                } else if (k == Model::KEY_LINK) {
                    this->link = v.GetString();
                } else if (k == Model::KEY_TEXT) {
                    this->text = v.GetString();
                } else if (k == Model::KEY_FILENAME) {
                    this->filename = v.GetString();
                } else if (k == Model::KEY_LINKS && v.IsArray()) {
                    for (SizeType i = 0; i < v.Size(); i++) {
                        this->links.insert(v[i].GetString());
                    }
                }

            }
        }
    }

    const std::string &getFilename() const {
        return filename;
    }

    const std::set<std::string> &getLinks() const {
        return links;
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

    std::string serialize();
};


#endif //COMMON_MODEL_H
