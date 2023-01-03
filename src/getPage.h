#include <stdio.h>
#include <curl/curl.h>

#include <vector>
#include <string>

//for libcurl
namespace get_page {
    static size_t write_data(void *ptr, 
                             size_t size, 
                             size_t nmemb, 
                             void *stream);

    void getPage(const std::string& url, 
                 const std::vector<std::string>& headers, 
                 const std::string& outFile);
}
