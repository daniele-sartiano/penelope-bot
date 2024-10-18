//
// Created by lele on 31/07/19.
//

#include "Downloader.h"
#include <curl/curl.h>
#include <string>
#include <fstream>
#include <iostream>
#include <cstring>


size_t Downloader::write_data(void *ptr, size_t size, size_t nmemb, void *stream) {
    /*std::ofstream& outstream = *reinterpret_cast<std::ofstream*>(stream);
    outstream.write((const char*)ptr, size * nmemb);*/

    size_t written = fwrite(ptr, size, nmemb, (FILE *)stream);
    //return size*nmemb;
    return written;
}


bool Downloader::discard(Model& m) {
    CURL *curl;
    CURLcode res;
    long filetime = -1;

    curl = curl_easy_init();

    curl_easy_getinfo(curl, CURLINFO_FILETIME, &filetime);
    curl_easy_setopt(curl, CURLOPT_URL, m.get_link().c_str());
    curl_easy_setopt(curl, CURLOPT_USERAGENT, this->USER_AGENT.c_str());
    curl_easy_setopt(curl, CURLOPT_NOBODY, 1L );
    curl_easy_setopt(curl, CURLOPT_HEADER, 0L );
    curl_easy_setopt(curl, CURLOPT_FILETIME, 1L);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

    res = curl_easy_perform(curl);

    if(CURLE_OK == res) {
        res = curl_easy_getinfo(curl, CURLINFO_FILETIME, &filetime);
        //std::cout << filetime << std::endl;
        return ((CURLE_OK == res) && filetime > 0 && (m.get_timestamp() >= filetime));
    }
    return false;
}

std::string Downloader::download(std::string &directory) {
    std::string prefix = !directory.empty() ? directory.append("/") : "";
    std::vector<Model> downloader_models;

    for (auto m:this->models) {
        const clock_t begin_time = clock();
        CURL *curl;
        long filetime = -1;
        std::hash<std::string> hasher;
        auto hashed = hasher(m.get_link());

        std::string file_name;
        file_name.append(prefix);
        file_name.append("page.");
        file_name.append(std::to_string(hashed));
        file_name.append(".out");

        //std::cout << "filename --> " << file_name << std::endl;

        FILE  *pagefile;
        pagefile = fopen(file_name.c_str(), "wb");
        if (!pagefile) {
            std::cerr << "can not create file " << file_name << std::endl;
            exit(EXIT_FAILURE);
        }

        if (this->discard(m)) {
            std::cout << "discard " << m.get_link() << std::endl;
            continue;
        }

        curl = curl_easy_init();

        curl_easy_setopt(curl, CURLOPT_URL, m.get_link().c_str());
        curl_easy_getinfo(curl, CURLINFO_FILETIME, &filetime);
        curl_easy_setopt(curl, CURLOPT_USERAGENT, this->USER_AGENT.c_str());
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
        //curl_easy_setopt(curl, CURLOPT_WRITEDATA, &out_file);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, pagefile);
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, true);

        curl_easy_perform(curl); /* ignores error */

        auto res = curl_easy_getinfo(curl, CURLINFO_FILETIME, &filetime);

        char *ip;
        res = curl_easy_getinfo(curl, CURLINFO_PRIMARY_IP, &ip);

        fclose(pagefile);

        curl_easy_cleanup(curl);
        curl_global_cleanup();
        // int timestamp, std::string link, std::string text, std::string filename, std::set<std::string> links
        m.set_timestamp(filetime);
        m.set_filename(file_name);
        m.set_ip(ip);
        downloader_models.push_back(m);
        std::cout << "\tTime: " << float( clock () - begin_time ) /  CLOCKS_PER_SEC << " " << m.get_link().c_str() << std::endl;
    }
    this->models.clear();
    this->models = downloader_models;

    return Model::serialize_models(this->models);
}

std::vector<Model> Downloader::get_models() const {
    return models;
}
