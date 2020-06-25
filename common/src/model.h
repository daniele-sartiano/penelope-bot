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
#include <vector>

using namespace rapidjson;

class Model {
private:
    static constexpr const char *KEY_LINKS = "links";
    static constexpr const char *KEY_TIMESTAMP = "timestamp";
    static constexpr const char *KEY_LINK = "link";
    static constexpr const char *KEY_TEXT = "text";
    static constexpr const char *KEY_FILENAME = "filename";
    static constexpr const char *KEY_IP = "ip";
    static constexpr const char *KEY_MODELS = "models";

    long timestamp = 0;
    std::string link = "";
    std::string text = "";
    std::string filename = "";
    std::string ip = "";
    std::set<std::string> links;

public:

    Model(int timestamp, std::string link, std::string text, std::string filename, std::string ip,
          std::set<std::string> links) : timestamp(timestamp), link(std::move(link)), text(std::move(text)),
                                         filename(std::move(filename)),
                                         ip(std::move(ip)),
                                         links(std::move(links)) {}

    explicit Model(const std::string &serialized) {
        Document d;
        Value v;
        d.Parse(serialized.c_str());

        for (std::string k: {Model::KEY_TIMESTAMP, Model::KEY_LINK, Model::KEY_TEXT, Model::KEY_FILENAME, Model::KEY_IP,
                             Model::KEY_LINKS}) {
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
                } else if (k == Model::KEY_IP) {
                    this->ip = v.GetString();
                } else if (k == Model::KEY_LINKS && v.IsArray()) {
                    for (SizeType i = 0; i < v.Size(); i++) {
                        this->links.insert(v[i].GetString());
                    }
                }

            }
        }
    }

    static std::vector<Model> deserialize_models(const std::string &serialized);

    static std::string serialize_models(std::vector<Model>);

    void set_timestamp(long timestamp);

    void set_link(const std::string &link);

    void set_text(const std::string &text);

    void set_filename(const std::string &filename);

    void set_ip(const std::string &ip);

    void set_links(const std::set<std::string> &links);

    const std::string &get_ip() const {
        return ip;
    }

    const std::string &get_filename() const {
        return filename;
    }

    const std::set<std::string> &get_links() const {
        return links;
    }

    long get_timestamp() const {
        return timestamp;
    }

    const std::string &get_link() const {
        return link;
    }

    const std::string &get_text() const {
        return text;
    };

    std::string serialize();
};


#endif //COMMON_MODEL_H
