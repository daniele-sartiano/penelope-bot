//
// Created by lele on 31/07/19.
//

#include "Downloader.h"
#include <curl/curl.h>
#include <string>
#include <fstream>
#include <iostream>
#include <tuple>


size_t Downloader::write_data(void *ptr, size_t size, size_t nmemb, void *stream) {
    std::ofstream& outstream = *reinterpret_cast<std::ofstream*>(stream);
    outstream.write((const char*)ptr, size * nmemb);

    //size_t written = fwrite(ptr, size, nmemb, (FILE *)stream);
    return size*nmemb;
}


bool Downloader::discard() {
    CURL *curl;
    CURLcode res;
    long filetime = -1;

    curl = curl_easy_init();

    curl_easy_getinfo(curl, CURLINFO_FILETIME, &filetime);
    curl_easy_setopt(curl, CURLOPT_URL, this->link.c_str());
    curl_easy_setopt(curl, CURLOPT_USERAGENT, this->USER_AGENT.c_str());
    curl_easy_setopt(curl, CURLOPT_NOBODY, 1L );
    curl_easy_setopt(curl, CURLOPT_HEADER, 0L );
    curl_easy_setopt(curl, CURLOPT_FILETIME, 1L);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

    res = curl_easy_perform(curl);

    if(CURLE_OK == res) {
        res = curl_easy_getinfo(curl, CURLINFO_FILETIME, &filetime);
        std::cout << filetime << std::endl;
        return ((CURLE_OK == res) && (this->last_seen >= filetime));
    }
    return false;
}

std::tuple<std::string, long, bool> Downloader::download(std::string &directory) {
    CURL *curl;
    std::string file_name;
    long filetime = -1;
    std::string prefix = !directory.empty() ? directory.append("/") : "";
    file_name.append(prefix);
    file_name.append("page.");

    file_name.append(this->link);
    file_name.append(".out");

    std::cout << "filename --> " << file_name << std::endl;

    std::ofstream out_file;
    out_file.open(file_name);

    if (this->discard()) {
        std::cout << "discard " << this->link << std::endl;
        return std::make_tuple(file_name, this->last_seen, false);
    }

    curl = curl_easy_init();

    curl_easy_setopt(curl, CURLOPT_URL, this->link.c_str());
    curl_easy_getinfo(curl, CURLINFO_FILETIME, &filetime);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, this->USER_AGENT.c_str());
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &out_file);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, true);

    curl_easy_perform(curl); /* ignores error */

    auto res = curl_easy_getinfo(curl, CURLINFO_FILETIME, &filetime);

    out_file.close();

    curl_easy_cleanup(curl);

    return std::make_tuple(file_name, filetime, true);
}
