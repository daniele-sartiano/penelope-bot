//
// Created by lele on 13/05/20.
//

#include "Parser.h"
#include <curl/curl.h>
#include <iterator>
#include <sstream>

void LexborParser::parse(const std::string &filename) {
    lxb_status_t status;
    const lxb_char_t *tag_name;
    lxb_html_document_t *document;
    lxb_dom_collection_t *collection;
    lxb_dom_element_t *element;

    std::ifstream in(filename);
    std::string contents((std::istreambuf_iterator<char>(in)),
                         std::istreambuf_iterator<char>());


    /*const lxb_char_t html[] = "<div a=b><span></div><div x=z></div>";
    size_t html_szie = sizeof(html) - 1;*/

    document = lxb_html_document_create();
    if (document == nullptr) {
        exit(EXIT_FAILURE);
    }

    status = lxb_html_document_parse(document, reinterpret_cast<const lxb_char_t *>(contents.c_str()), strlen(contents.c_str()));
    //status = lxb_html_document_parse(document, html, html_szie);
    if (status != LXB_STATUS_OK) {
        exit(EXIT_FAILURE);
    }

    collection = lxb_dom_collection_make(&document->dom_document, 128);
    if (collection == NULL) {
        exit(EXIT_FAILURE);
    }

    status = lxb_dom_elements_by_tag_name(lxb_dom_interface_element(document->body),
                                          collection, (const lxb_char_t *) "a", 1);
    if (status != LXB_STATUS_OK) {
        exit(EXIT_FAILURE);
    }

    for (size_t i = 0; i < lxb_dom_collection_length(collection); i++) {
        element = lxb_dom_collection_element(collection, i);

        lxb_dom_node_t *node = lxb_dom_interface_node(element);
        lexbor_str_t str = {0};
        status = lxb_html_serialize_pretty_str(node, LXB_HTML_SERIALIZE_OPT_UNDEF,
                                              0, &str);

        if (status != LXB_STATUS_OK) {
            exit(EXIT_FAILURE);
        }

        printf("--> %s| %d\n", str.data, str.length);
    }



    /*tag_name = lxb_dom_element_qualified_name(lxb_dom_interface_element(document->body),
                                              nullptr);

    printf("Element tag name: %s\n", tag_name);*/

    lxb_html_document_destroy(document);

}

std::string Parser::parse() {

    std::vector<Model> parser_models;

    for (auto m: this->models) {
        std::string url = m.get_link();
        std::string filename = m.get_filename();

        std::cout << "Elaborating " << filename << std::endl;

        std::ifstream in(filename, std::ios::in | std::ios::binary);
        if (!in) {
            std::cerr << "File " << filename << " not found!\n";
            continue;
        }

        std::string contents;
        in.seekg(0, std::ios::end);
        contents.resize(in.tellg());
        in.seekg(0, std::ios::beg);
        in.read(&contents[0], contents.size());
        in.close();

        GumboOutput* output = gumbo_parse(contents.c_str());
        std::set<std::string> links = this->extract_links(output->root, url);
        std::string text = this->clean_text(output->root);
        gumbo_destroy_output(&kGumboDefaultOptions, output);
        m.set_text(text);
        m.set_links(links);

        if(remove( filename.c_str()) != 0) {
            std::cerr <<  "Error deleting file " << filename << std::endl;
        }

        parser_models.push_back(m);
    }

    this->models.clear();
    this->models = parser_models;

    return Model::serialize_models(this->models);
}

std::set<std::string> Parser::extract_links(GumboNode *node, std::string& url) {
    std::set<std::string> links;
    if (node->type != GUMBO_NODE_ELEMENT) {
        return links;
    }
    GumboAttribute* href;
    if (node->v.element.tag == GUMBO_TAG_A &&
        (href = gumbo_get_attribute(&node->v.element.attributes, "href"))) {
        //std::cerr << "was " << href->value << " now is " << this->transform_link(href->value, url) << std::endl;
        links.insert(this->transform_link(href->value, url));
    }
    GumboVector* children = &node->v.element.children;
    for (unsigned int i = 0; i < children->length; ++i) {
        std::set<std::string> v = this->extract_links(static_cast<GumboNode*>(children->data[i]), url);
        links.insert(v.begin(), v.end());
    }
    return links;
}


std::string Parser::clean_text(GumboNode* node) {
    if (node->type == GUMBO_NODE_TEXT) {
        return std::string(node->v.text.text);
    } else if (node->type == GUMBO_NODE_ELEMENT &&
               node->v.element.tag != GUMBO_TAG_SCRIPT &&
               node->v.element.tag != GUMBO_TAG_STYLE) {
        std::string contents;
        GumboVector* children = &node->v.element.children;
        for (unsigned int i = 0; i < children->length; ++i) {
            const std::string text = this->clean_text((GumboNode*) children->data[i]);
            if (i != 0 && !text.empty()) {
                contents.append(" ");
            }
            std::string trim_text = trim(text);
            if (!trim_text.empty()) {
                //std::cout << "-->" << trim_text << "<--" << std::endl;
                contents.append(trim_text);
            }
        }
        return contents;
    } else {
        return "";
    }
}

inline std::string Parser::trim(const std::string &s) {
    auto wsfront=std::find_if_not(s.begin(),s.end(),[](int c){return std::isblank(c);});
    auto wsback=std::find_if_not(s.rbegin(),s.rend(),[](int c){return std::isblank(c);}).base();
    return (wsback<=wsfront ? std::string() : std::string(wsfront,wsback));
}

inline std::string Parser::transform_link(const std::string &link, const std::string &url) {

    if (link.rfind("http://", 0) == 0 || link.rfind("https://", 0) == 0) {
        return link;
    }
    std::string ns;
    ns.append(url);
    if (url.back() != '/') {
        ns.append("/");
    }

    if  (link.front() == '/') {
        ns.append(link.substr(1, link.size()));
    } else {
        ns.append(link);
    }
    return ns;
}

std::vector<Model> Parser::get_models() const {
    return models;
}
