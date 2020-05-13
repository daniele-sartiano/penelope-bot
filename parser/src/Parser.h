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

class LexborParser {
public:
    void parse(const std::string &filename);
};

class Parser {
public:
    void parse(const std::string &filename);
private:
    void extract_links(GumboNode* node);
};

#endif //PARSER_PARSER_H
