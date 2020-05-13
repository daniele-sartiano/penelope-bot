#include <iostream>
#include "src/Parser.h"

int main() {
    std::cout << "Hello, World!" << std::endl;

    Parser *p = new Parser();
    p->parse("/projects/penelope-bot/downloader/cmake-build-debug/output/page.www.sartiano.info.out");

    return EXIT_SUCCESS;
}
