#ifndef COMMON_H
#define COMMON_H

#include <memory>
#include <utility>
#include <algorithm>
#include <thread>
#include <mutex>

#include <ctime>
#include <chrono>

#include <cstdio>
#include <iostream>
#include <fstream>

#include <cstring>
#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>

// external includes
// #define CURL_STATICLIB
#include <curl/curl.h>

#define DEBUG 0

// ===== STRUCTS ===============================================================

// holds a type of train, such as an F train with dirID 1
struct Train {
    Train(const std::string& name, int dirID):
        name(name), dirID(dirID) {}

    std::string name; //same as name member in line
    int dirID; // 0 for north, 1 for south

    
};
bool operator==(const Train& lhs, const Train& rhs);
bool operator<(const Train& lhs, const Train& rhs); 

// holds an arrival to a station, which is a train, its headsign, and the time that it arrives
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

// represents an SQLite table, holds the name of the table and the column headers of the table
struct Table {
    Table(const std::string& name, 
          const std::vector<std::pair<std::string, std::string>> columns):
        name(name), columns(columns) {}

    const std::string name;
    const std::vector<std::pair<std::string, std::string>> columns; //name, data type
};

struct NSThash {
    size_t operator()(const Train& train) const {
        return std::hash<std::string>{}(
            train.name + std::to_string(train.dirID)
        );
    }

    size_t operator()(const Arrival& arrival) const {
        return std::hash<std::string>{}(
            arrival.train->name + std::to_string(arrival.train->dirID) + arrival.ftime
        );
    }

};

// ===== CONSTANTS =============================================================

namespace constant {
    const uint8_t THREADS = 8;
    const std::string LOGGER_NAME = "subway-logger";
    const std::string DB_NAME = "nyc-subway-tracker.db";

    const unsigned int CONNECTION_TIMEOUT_WAIT = 5;

    const size_t STATION_RESERVE_NEARBY = 9;

    const size_t LINE_RESERVE_STATION = 40;

    const size_t SUBWAY_RESERVE_LINES = 26;
    const size_t SUBWAY_RESERVE_ALLSTATIONS = 496;
    const size_t SUBWAY_RESERVE_TRAINTYPES = 52;

    const std::unordered_map<std::string, std::string> SHUTTLE_NAMES{ //short id (valid url), name
        {"H", "SR"},  // Franklin Av
        {"FS", "SF"}, // Rockaway Park
        {"GS", "S"},  // 42nd St
        {"SI", "SIR"} // Staten Island Railway (not a shuttle)
    };

    const std::string STATION_URL = "https://otp-mta-prod.camsys-apps.com/otp/routers/default/nearby?stops=MTASBWY:";
    const std::string STATION_API_KEY = "Z276E3rCeTzOQEoBPPN4JCEc6GfvdnYE";

    const std::string LINE_URL = "https://collector-otp-prod.camsys-apps.com/schedule/MTASBWY/stopsForRoute?routeId=MTASBWY:";
    const std::string LINE_API_KEY = "qeqy84JE7hUKfaI0Lxm2Ttcm6ZA0bYrP";

    const std::string SUBWAY_URL = "https://otp-mta-prod.camsys-apps.com/otp/routers/default/index/routes";
    const std::string SUBWAY_API_KEY = "Z276E3rCeTzOQEoBPPN4JCEc6GfvdnYE";

    //==========================================================================

    // snapshot
    const std::string SNAPSHOT_TABLE_NAME = "Snapshots";
    const std::vector<std::pair<std::string, std::string>> SNAPSHOT_COLUMNS = { // ex) Subway Stations
        std::make_pair("Subway_Snapshot", "TEXT"), // ex) Subway_1673908573

        std::make_pair("Type", "TEXT"), // ex) Subway

        std::make_pair("Time", "INTEGER"), // ex) 2024-01-06-15:19:25
        std::make_pair("Time_Unix", "INTEGER"), // ex) 1673908573
    };
    const Table SNAPSHOT_TABLE(
        SNAPSHOT_TABLE_NAME,
        SNAPSHOT_COLUMNS
    );

    // name is something like 
    const std::vector<std::pair<std::string, std::string>> SUBWAY_COLUMNS = {
        std::make_pair("Table_Name", "TEXT"),
        std::make_pair("Type", "TEXT"),
        std::make_pair("Name", "TEXT")
    };

    // ex) Subway_Stations_1673908573, subway_station_snapshot
    const std::vector<std::pair<std::string, std::string>> LINES_COLUMNS = { 
        std::make_pair("StopID_and_Time", "TEXT"), // ex) Station_G14_1673908573
        std::make_pair("StopID", "TEXT"), // ex) G14
        std::make_pair("Name", "TEXT"), // ex) Roosevelt Av

        std::make_pair("Time_Checked", "INTEGER"), // ex) 2024-01-06-15:19:25
        std::make_pair("Time_Checked_Unix", "INTEGER") // ex) 1673908573
    };

    // ex) Station_G14_1673908573, station_snapshot
    const std::vector<std::pair<std::string, std::string>> STATION_COLUMNS = { 
        std::make_pair("Name_and_DirID_and_Time", "TEXT"), // ex) Nearby_F_1673909000

        std::make_pair("Name", "TEXT"), // ex) F
        std::make_pair("Direction_ID", "INTEGER"), // ex) 1
        std::make_pair("Headsign", "TEXT"), // ex) Manhattan

        std::make_pair("Time_Checked", "TEXT"), // ex) 2024-01-06-15:19:25
        std::make_pair("Time_Until_Arrival", "TEXT"), //ex) 05:33
        std::make_pair("Time_of_Arrival", "TEXT"), // ex) 2024-01-06-15:19:25

        std::make_pair("Time_Checked_Unix", "TEXT"),
        std::make_pair("Time_Until_Arrival_Unix", "TEXT"), //ex) 05:33
        std::make_pair("Time_of_Arrival_Unix", "TEXT"), // ex) 2024-01-06-15:19:25
    };

    // For each snapshot:
    // SNAPSHOTS (one)
    //          |--> SUBWAY (one)
    //                     |--> STATIONS (mul)
    //                                  |--> STATION_DATA (data)
    //                     |--> LINES (mul)
    //                                  |--> STATIONS (mul) 
    //                                               |--> STATION_DATA (data)
}

// ===== COMMON FUNCTIONS ======================================================

namespace common {
    enum {
        NORMAL,
        SQLITE,
        MINSEC,
        DAY_TIME
    };
    // takes a time and returns it in a string (no whitespace) format
    std::string formatTime(time_t* time = nullptr, int mode = NORMAL);
    void panic(const std::string& filename, const std::string& funcname, const std::string& misc="");
}

// ===== CURL FUNCTIONS=========================================================

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