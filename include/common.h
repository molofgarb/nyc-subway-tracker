#ifndef COMMON_H
#define COMMON_H

#include <memory>
#include <utility>
#include <thread>
#include <mutex>

#include <ctime>
#include <chrono>

#include <cstdio>
#include <iostream>
#include <fstream>

#include <string>
#include <vector>
#include <map>
#include <set>

// external includes
// #define CURL_STATICLIB
#include <curl/curl.h>

#define DEBUG 0

namespace constant {
    const uint8_t THREADS = 8;
    const std::string LOGGER_NAME = "subway-logger";
    const std::string DB_NAME = "nyc-subway-tracker.db";

    const std::map<std::string, std::string> SHUTTLE_NAMES{ //short id (valid url), name
        {"H", "SR"}, //Franklin Av
        {"FS", "SF"}, //Rockaway Park
        {"GS", "S"}, //42nd St
        {"SI", "SIR"} //Staten Island Railway (not a shuttle)
    };

    const std::string STATION_URL = "https://otp-mta-prod.camsys-apps.com/otp/routers/default/nearby?stops=MTASBWY:";
    const std::string STATION_API_KEY = "Z276E3rCeTzOQEoBPPN4JCEc6GfvdnYE";

    const std::string LINE_URL = "https://collector-otp-prod.camsys-apps.com/schedule/MTASBWY/stopsForRoute?routeId=MTASBWY:";
    const std::string LINE_API_KEY = "qeqy84JE7hUKfaI0Lxm2Ttcm6ZA0bYrP";

    const std::string SUBWAY_URL = "https://otp-mta-prod.camsys-apps.com/otp/routers/default/index/routes";
    const std::string SUBWAY_API_KEY = "Z276E3rCeTzOQEoBPPN4JCEc6GfvdnYE";
}

// =============================================================================

struct Train {
    Train(const std::string& name, int dirID):
        name(name), dirID(dirID) {}

    std::string name; //same as name member in line
    int dirID; // 0 for north, 1 for south
};
bool operator==(const Train& lhs, const Train& rhs);
bool operator<(const Train& lhs, const Train& rhs); //(needed for std::map)

struct Arrival {
    Arrival(const Train* train = nullptr, std::string headsign = "", 
            std::time_t time = 0, std::string ftime = ""
    ):
        train(train), headsign(headsign), time(time), ftime(ftime) {}

    const Train* train;
    std::string headsign;
    std::time_t time; //time of arrival
    std::string ftime;
};

// =============================================================================

namespace common {
    // takes a time and returns it in a string (no whitespace) format
    std::string formatTime(time_t* time = nullptr, bool sqlite=false);
    void panic(const std::string& filename, const std::string& funcname, const std::string& misc="");
}

// =============================================================================

//for libcurl
namespace get_page {
    //used in libcurl to write data from webpage into file
    size_t write_data(void* ptr, 
                      size_t size, 
                      size_t nmemb, 
                      void* stream);

    //uses libcurl to get a webpage
    int get_page(const std::string& url, 
                 const std::vector<std::string>& headers, 
                 std::string& data);

}

#endif