#include <iostream>
#include <stdio.h>
#include <curl/curl.h>

#include <string>

#include "getPage.h"


// from libcurl url2file (https://curl.se/libcurl/c/url2file.html)
static size_t get_page::write_data(void *ptr, 
                                   size_t size, 
                                   size_t nmemb, 
                                   void *stream) {
    size_t written = fwrite(ptr, size, nmemb, (FILE*)stream);
    return written;
}

void get_page::getPage(const std::string& url, 
                       const std::vector<std::string>& headers, 
                       const std::string& outFile) {
    CURL* curl = curl_easy_init();
    FILE* file;
    CURLcode res;
    
    curl_global_init(CURL_GLOBAL_DEFAULT);

    //configure headers
    struct curl_slist* full_header = NULL;
    for (const std::string header : headers) {
        full_header = curl_slist_append(full_header, header.data());
    }
    
    //get page
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url.data());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, full_header);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, get_page::write_data);
        // curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
        // curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1L);
        file = fopen(outFile.data(), "wb");
        if (file) {
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, file);
            res = curl_easy_perform(curl);
            std::cout << "<DEBUG> CURLcode: " << res << std::endl;
            fclose(file);
        }
    }

    //clean
    curl_slist_free_all(full_header);
    curl_easy_cleanup(curl);
    curl_global_cleanup();
}

