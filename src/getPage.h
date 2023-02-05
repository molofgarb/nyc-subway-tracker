#include <iostream>
#include <fstream>

#include <stdio.h>
#include <curl/curl.h>

#include <map>
#include <vector>
#include <string>

#include <algorithm>

namespace constant {
    const std::map<std::string, std::string> SHUTTLE_NAMES{ //short id (valid url), name
        {"H", "SR"}, //Franklin Av
        {"FS", "SF"}, //Rockaway Park
        {"GS", "S"}, //42nd St
        {"SI", "SIR"} //Staten Island Railway (not a shuttle)
    };

    const std::string STATION_URL = "https://otp-mta-prod.camsys-apps.com/otp/routers/default/nearby?stops=MTASBWY%3A";
    const std::string STATION_API_KEY = "Z276E3rCeTzOQEoBPPN4JCEc6GfvdnYE";

    const std::string LINE_URL = "https://collector-otp-prod.camsys-apps.com/schedule/MTASBWY/stopsForRoute?apikey=qeqy84JE7hUKfaI0Lxm2Ttcm6ZA0bYrP&&routeId=MTASBWY:";
    const std::string LINE_API_KEY = "qeqy84JE7hUKfaI0Lxm2Ttcm6ZA0bYrP";

    const std::string SUBWAY_URL = "https://otp-mta-prod.camsys-apps.com/otp/routers/default/index/routes";
    const std::string SUBWAY_API_KEY = "Z276E3rCeTzOQEoBPPN4JCEc6GfvdnYE";
}

//for libcurl
namespace get_page {
    //used in libcurl to write data from webpage into file
    static size_t write_data(void *ptr, 
                             size_t size, 
                             size_t nmemb, 
                             void *stream);

    //uses libcurl to get a webpage
    void getPage(const std::string& url, 
                 const std::vector<std::string>& headers, 
                 const std::string& outFile);

}
