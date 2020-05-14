//
// Created by lele on 13/05/20.
//

#include <iostream>
#include "Parser.h"

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

void Parser::parse(const std::string &filename) {
    std::ifstream in(filename, std::ios::in | std::ios::binary);
    if (!in) {
        std::cerr << "File " << filename << " not found!\n";
        exit(EXIT_FAILURE);
    }

    std::string contents;
    in.seekg(0, std::ios::end);
    contents.resize(in.tellg());
    in.seekg(0, std::ios::beg);
    in.read(&contents[0], contents.size());
    in.close();

    GumboOutput* output = gumbo_parse(contents.c_str());
    this->extract_links(output->root);
    gumbo_destroy_output(&kGumboDefaultOptions, output);
}

void Parser::extract_links(GumboNode *node) {
    if (node->type != GUMBO_NODE_ELEMENT) {
        return;
    }
    GumboAttribute* href;
    if (node->v.element.tag == GUMBO_TAG_A &&
        (href = gumbo_get_attribute(&node->v.element.attributes, "href"))) {
        std::cout << href->value << std::endl;
    }

    GumboVector* children = &node->v.element.children;
    for (unsigned int i = 0; i < children->length; ++i) {
        this->extract_links(static_cast<GumboNode*>(children->data[i]));
    }
}
