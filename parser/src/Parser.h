//
// Created by lele on 13/05/20.
//

#ifndef PARSER_PARSER_H
#define PARSER_PARSER_H


#include <string>
#include <fstream>
#include <lexbor/html/parser.h>
#include <lexbor/dom/interfaces/element.h>
#include <lexbor/html/html.h>
#include <gumbo.h>
#include <algorithm>
#include <iostream>
#include <vector>
#include <tuple>
#include <model.h>


class LexborParser {
public:
    void parse(const std::string &filename);
};

class Parser {
    std::vector<Model> models;
public:
    Parser(const std::string& serialized) {
        this->models = Model::deserialize_models(serialized);
    }
    std::string parse();
    std::vector<Model> get_models() const;

private:
    std::set<std::string> extract_links(GumboNode* node, std::string& url);
    std::string clean_text(GumboNode* node);
    std::string trim(const std::string &s);
    std::string transform_link(const std::string &link, const std::string &url);
};

#endif //PARSER_PARSER_H
