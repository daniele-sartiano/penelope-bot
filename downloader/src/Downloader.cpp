//
// Created by lele on 31/07/19.
//

#include <curl/curl.h>
#include <string>
#include <fstream>
#include "Downloader.h"

size_t Downloader::write_data(void *ptr, size_t size, size_t nmemb, void *stream) {
    std::ofstream& outstream = *reinterpret_cast<std::ofstream*>(stream);
    outstream.write((const char*)ptr, size * nmemb);

    //size_t written = fwrite(ptr, size, nmemb, (FILE *)stream);
    return size*nmemb;
}

void Downloader::download() {
    CURL *curl;

    std::string file_name = "page.";
    file_name.append(this->url);
    file_name.append(".out");

    std::ofstream out_file;
    out_file.open(file_name);

    curl = curl_easy_init();

    curl_easy_setopt(curl, CURLOPT_URL, this->url);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &out_file);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, true);

    curl_easy_perform(curl); /* ignores error */

    out_file.close();

    curl_easy_cleanup(curl);
}
