#include <iostream>

#include <string>
#include <vector>
#include <map>

#include "curl/include/curl/curl.h"
#include <cstdio>


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

    const std::string DB_NAME = "nyc-subway-tracker.db";

    const std::vector<std::pair<std::string, std::string>> STATION_COLUMNS = { // ex) G14 1673908573
        std::make_pair("Name", "TINYTEXT"), // ex) F
        std::make_pair("DirID", "BIT(1)"), // ex) 1
        std::make_pair("Time of Arrival", "BIGINT(16)") // ex) 1673909000
    };


    const std::vector<std::pair<std::string, std::string>> SUBWAY_STATIONS_COLUMNS = { // ex) Stations 1673908573
        std::make_pair("StopID & Time", "TINYTEXT"), // ex) G14 1673908573
        std::make_pair("StopID", "TINYTEXT"), // ex) G14
        std::make_pair("Name", "TINYTEXT") // ex) Roosevelt Av
    };

    const std::vector<std::pair<std::string, std::string>> SNAPSHOT_COLUMNS = { // ex) Subway Stations
        std::make_pair("Subway Snapshot", "TINYTEXT"), // ex) Stations 1673908573
        std::make_pair("Time", "BIGINT(16)"), // ex) 1673908573
    };

    // const std::vector<std::pair<std::string, std::string>> LINE_COLUMNS = { // ex) F 1673908573
    //     std::make_pair("StopID & Time", "TINYTEXT")
    // };

    // const std::vector<std::pair<std::string, std::string>> SUBWAY_LINES_COLUMNS = { // ex) Lines 1673908573

    // }

    // const std::vector<std::pair<std::string, std::string>> SNAPSHOT_LINES_COLUMNS = { //ex) Subway Snapshots
    //     std::make_pair("Subway Snapshot", "TIME(0)"), // ex) Stations 1673908573 ...OR... Lines 1673908573
    // };
}

//for libcurl
namespace get_page {
    //used in libcurl to write data from webpage into file
    static size_t write_data(void* ptr, 
                             size_t size, 
                             size_t nmemb, 
                             void* stream);

    //uses libcurl to get a webpage
    int get_page(const std::string& url, 
                 const std::vector<std::string>& headers, 
                 std::string& data);

}
