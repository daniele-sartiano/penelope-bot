#include <iostream>
#include <cassandra.h>
#include "src/DataManager.h"


int main() {
    std::cout << "Hello, World!" << std::endl;

    auto *d = new DataManager("127.0.0.1");
    const Model m(R"({"timestamp": 1590067123, "link": "www.unipi.it", "text": "prova prova prova due"})");
    d->insert_model(m);



    return 0;
}
