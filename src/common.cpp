#include <iostream>

#include <cstdio>
#include <curl/curl.h>

#include <string>
#include <vector>

#include "common.h"


static size_t get_page::write_data(void* dataptr,     //pointer to data from curl
                                   size_t size,       //size of each data element
                                   size_t nmemb,      //num of data elements
                                   void* outstream) { //data output stream
    ((std::string*)outstream)->append((char*)dataptr, size * nmemb);
    return size * nmemb;
}

int get_page::get_page(const std::string& url, 
                       const std::vector<std::string>& headers, 
                       std::string& data) {
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

        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &data);
        res = curl_easy_perform(curl);
        // std::cout << "<DEBUG getPage.cpp> CURLcode: " << res << "\t URL: " << url << std::endl;
    }
    else return 1;

    //clean
    curl_slist_free_all(full_header);
    curl_easy_cleanup(curl);
    curl_global_cleanup();
    return 0;
}