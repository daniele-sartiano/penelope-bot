//
// Created by lele on 31/07/19.
//

#include "Downloader.h"
#include <curl/curl.h>
#include <string>
#include <fstream>
#include <iostream>
#include <cstring>
#include <ctime>


struct TransferContext {
    FILE *fp;
    Model model;
    std::string filename;
};


static size_t write_data_cb(void *ptr, size_t size, size_t nmemb, void *stream) {
    return fwrite(ptr, size, nmemb, (FILE *)stream);
}


static CURL* make_easy_handle(const std::string &url, const std::string &user_agent,
                              TransferContext *ctx) {
    CURL *easy = curl_easy_init();
    curl_easy_setopt(easy, CURLOPT_URL, url.c_str());
    curl_easy_setopt(easy, CURLOPT_FILETIME, 1L);
    curl_easy_setopt(easy, CURLOPT_USERAGENT, user_agent.c_str());
    curl_easy_setopt(easy, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(easy, CURLOPT_WRITEFUNCTION, write_data_cb);
    curl_easy_setopt(easy, CURLOPT_WRITEDATA, ctx->fp);
    curl_easy_setopt(easy, CURLOPT_CONNECTTIMEOUT, 30L);
    curl_easy_setopt(easy, CURLOPT_TIMEOUT, 300L);
    curl_easy_setopt(easy, CURLOPT_NOSIGNAL, 1L);
    curl_easy_setopt(easy, CURLOPT_PRIVATE, ctx);
    return easy;
}


std::string Downloader::download(const std::string &directory) {
    std::string prefix = !directory.empty() ? (directory + "/") : "";
    std::vector<Model> downloader_models;

    if (this->models.empty()) {
        return Model::serialize_models(downloader_models);
    }

    CURLM *multi = curl_multi_init();
    size_t next_url = 0;
    const size_t total = this->models.size();

    auto enqueue_next = [&]() {
        if (next_url >= total) return;
        Model &m = this->models[next_url++];
        std::hash<std::string> hasher;
        auto hashed = hasher(m.get_link());

        auto *ctx = new TransferContext{nullptr, m,
            prefix + "page." + std::to_string(hashed) + ".out"};

        ctx->fp = fopen(ctx->filename.c_str(), "wb");
        if (!ctx->fp) {
            std::cerr << "cannot create file " << ctx->filename << std::endl;
            delete ctx;
            return;
        }

        CURL *easy = make_easy_handle(m.get_link(), this->USER_AGENT, ctx);
        curl_multi_add_handle(multi, easy);
    };

    size_t initial = std::min(this->max_concurrent, total);
    for (size_t i = 0; i < initial; i++) {
        enqueue_next();
    }

    int still_running = 0;
    do {
        CURLMcode mc = curl_multi_perform(multi, &still_running);
        if (mc != CURLM_OK) {
            std::cerr << "curl_multi_perform failed: " << curl_multi_strerror(mc) << std::endl;
            break;
        }

        if (still_running) {
            curl_multi_poll(multi, nullptr, 0, 1000, nullptr);
        }

        CURLMsg *msg;
        int msgs_left;
        while ((msg = curl_multi_info_read(multi, &msgs_left))) {
            if (msg->msg != CURLMSG_DONE) continue;

            CURL *easy = msg->easy_handle;
            TransferContext *ctx = nullptr;
            curl_easy_getinfo(easy, CURLINFO_PRIVATE, &ctx);

            if (ctx->fp) {
                fclose(ctx->fp);
                ctx->fp = nullptr;
            }

            if (msg->data.result == CURLE_OK) {
                long filetime = -1;
                char *ip = nullptr;
                curl_easy_getinfo(easy, CURLINFO_FILETIME, &filetime);
                curl_easy_getinfo(easy, CURLINFO_PRIMARY_IP, &ip);

                long ts = filetime > 0 ? filetime : std::time(nullptr);
                ctx->model.set_timestamp(ts);
                ctx->model.set_filename(ctx->filename);
                ctx->model.set_ip(ip != nullptr ? ip : "");
                downloader_models.push_back(ctx->model);
            } else {
                std::cerr << "download failed for " << ctx->model.get_link() << ": "
                          << curl_easy_strerror(msg->data.result) << std::endl;
            }

            curl_multi_remove_handle(multi, easy);
            curl_easy_cleanup(easy);
            delete ctx;

            enqueue_next();
        }
    } while (still_running || next_url < total);

    curl_multi_cleanup(multi);

    this->models = downloader_models;
    return Model::serialize_models(this->models);
}


std::vector<Model> Downloader::get_models() const {
    return models;
}
