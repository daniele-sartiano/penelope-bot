//
// Created by lele on 22/05/20.
//

#include "model.h"

std::string Model::serialize() {

    StringBuffer sb = nullptr;
    Writer<StringBuffer> writer(sb);
    writer.StartObject();

    for (std::string k: {Model::KEY_TIMESTAMP, Model::KEY_LINK, Model::KEY_TEXT, Model::KEY_FILENAME, Model::KEY_LINKS}) {
        writer.Key(k.c_str());
        if (k == Model::KEY_TIMESTAMP) {
            writer.Int64(this->timestamp);
        } else if (k == Model::KEY_LINK) {
            writer.String(this->link.c_str());
        } else if (k == Model::KEY_TEXT) {
            writer.String(this->text.c_str());
        } else if (k == Model::KEY_FILENAME) {
            writer.String(this->filename.c_str());
        } else if (k == Model::KEY_LINKS) {
            writer.StartArray();
            for (const std::string& s : this->links) {
                writer.String(s.c_str());
            }
            writer.EndArray();
        }
    }

    writer.EndObject();
    return sb.GetString();
}

void Model::setTimestamp(int timestamp) {
    Model::timestamp = timestamp;
}

void Model::setLink(const std::string &link) {
    Model::link = link;
}

void Model::setText(const std::string &text) {
    Model::text = text;
}

void Model::setFilename(const std::string &filename) {
    Model::filename = filename;
}

void Model::setLinks(const std::set<std::string> &links) {
    Model::links = links;
}
