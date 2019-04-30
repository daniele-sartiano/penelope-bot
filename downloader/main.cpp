#include <iostream>
#include <fstream>
#include <pthread.h>
#include <curl/curl.h>


static size_t write_data(void *ptr, size_t size, size_t nmemb, void *stream)
{
    std::ofstream& outstream = *reinterpret_cast<std::ofstream*>(stream);
    outstream.write((const char*)ptr, size * nmemb);

    //size_t written = fwrite(ptr, size, nmemb, (FILE *)stream);
    return size*nmemb;
}


static void *pull_one_url(void *url)
{
    CURL *curl;

    std::string file_name = "page.";
    file_name.append((char*)url);
    file_name.append(".out");

    std::ofstream out_file;
    out_file.open(file_name);

    curl = curl_easy_init();

    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &out_file);

    curl_easy_perform(curl); /* ignores error */

    out_file.close();

    curl_easy_cleanup(curl);

    return NULL;
}


int main() {

    std::cout << "Hello, World!" << std::endl;

    int NUMT = 3;
    pthread_t tid[NUMT];
    int error;

    const char * const urls[] = {
            "www.sartiano.info",
            "www.repubblica.it",
            "www.corriere.it"
    };

    curl_global_init(CURL_GLOBAL_ALL);

    for (int i=0; i<NUMT; i++) {
        error = pthread_create(
                &tid[i],
                NULL,
                pull_one_url,
                (void*)urls[i]);
        if (0 != error) {
            fprintf(stderr, "Could't run thread number %d, errno %d\n", i, error);
        } else {
            fprintf(stderr,  "Thread %d,  gets %s\n", i, urls[i]);
        }
    }

    /* now wait for all threads to terminate */
    for(int i=0; i< NUMT; i++) {
        error = pthread_join(tid[i], NULL);
        fprintf(stderr, "Thread %d terminated\n", i);
    }

    return 0;
}