// Standalone benchmark binary for the C++ Downloader.
//
// Reads configuration from environment:
//   SERVER_URL      - base URL (e.g. http://server:8888)
//   MAX_CONCURRENT  - concurrent downloads (default: Downloader::DEFAULT_MAX_CONCURRENT)
//   SCENARIOS       - comma-separated list of URL counts (default: "10,50,100,200")
//   DOWNLOAD_DIRECTORY - where to write files (default: /tmp)
//
// Builds a JSON payload matching the Model protocol, runs Downloader::download(),
// and prints wall-clock time per scenario.

#include "src/Downloader.h"
#include <chrono>
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>


static std::string getenv_or(const char *name, const std::string &fallback) {
    const char *v = getenv(name);
    return v != nullptr ? std::string(v) : fallback;
}


static std::vector<int> parse_scenarios(const std::string &spec) {
    std::vector<int> out;
    std::stringstream ss(spec);
    std::string token;
    while (std::getline(ss, token, ',')) {
        if (!token.empty()) out.push_back(std::stoi(token));
    }
    return out;
}


static std::string build_payload(const std::string &base_url, int num_urls) {
    std::ostringstream ss;
    ss << "{\"models\":[";
    for (int i = 0; i < num_urls; i++) {
        if (i > 0) ss << ",";
        ss << "{\"timestamp\":0,\"link\":\"" << base_url << "/page/" << i
           << "\",\"text\":\"\",\"filename\":\"\",\"ip\":\"\",\"links\":[]}";
    }
    ss << "]}";
    return ss.str();
}


int main() {
    std::string server_url = getenv_or("SERVER_URL", "http://127.0.0.1:8888");
    std::string directory = getenv_or("DOWNLOAD_DIRECTORY", "/tmp");
    std::string scenarios_spec = getenv_or("SCENARIOS", "10,50,100,200");

    size_t max_concurrent = Downloader::DEFAULT_MAX_CONCURRENT;
    const char *mc_env = getenv("MAX_CONCURRENT");
    if (mc_env != nullptr) {
        long v = std::strtol(mc_env, nullptr, 10);
        if (v > 0) max_concurrent = static_cast<size_t>(v);
    }

    auto scenarios = parse_scenarios(scenarios_spec);

    std::cout << "===========================================================" << std::endl;
    std::cout << "BENCHMARK: C++ Downloader (curl_multi)" << std::endl;
    std::cout << "server=" << server_url << "  max_concurrent=" << max_concurrent << std::endl;
    std::cout << "===========================================================" << std::endl;
    std::cout << "  URLs    Wall time" << std::endl;
    std::cout << "-----------------------------------------------------------" << std::endl;

    // Warm up
    {
        std::string payload = build_payload(server_url, 1);
        Downloader d(payload, max_concurrent);
        d.download(directory);
    }

    for (int n : scenarios) {
        std::string payload = build_payload(server_url, n);
        Downloader d(payload, max_concurrent);

        auto t0 = std::chrono::steady_clock::now();
        std::string result = d.download(directory);
        auto elapsed = std::chrono::duration<double>(
            std::chrono::steady_clock::now() - t0).count();

        size_t downloaded = d.get_models().size();
        std::cout << "  " << n << " URLs -> " << downloaded << " ok, "
                  << elapsed << " s" << std::endl;
    }

    std::cout << "-----------------------------------------------------------" << std::endl;
    return 0;
}
