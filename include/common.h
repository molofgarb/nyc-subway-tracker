#ifndef COMMON_H
#define COMMON_H

#include <pch.h>

#define DEBUG 0

class Station;
using st_ptr = std::shared_ptr<Station>;

// ===== STRUCTS ===============================================================

// holds a type of train, such as an F train with dirID 1
struct Train {
    Train(const std::string& name, int dirID):
        name(name), dirID(dirID) {}

    std::string name; //same as name member in line
    int dirID; // 0 for north, 1 for south
};
bool operator==(const Train& lhs, const Train& rhs);

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
          const std::vector<std::string>& columns):
        name(name), columns(columns) {}

    const std::string name;
    const std::vector<std::string> columns; //name, data type
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

    // memory reservations for data structures to reduce copy/swap cases
    const size_t STATION_RESERVE_NEARBY = 9;

    const size_t LINE_RESERVE_STATION = 40;

    const size_t SUBWAY_RESERVE_LINES = 26;
    const size_t SUBWAY_RESERVE_ALL_STATIONS = 496;
    const size_t SUBWAY_RESERVE_TRAIN_TYPES = 52;

    const size_t SQLITE_RESERVE_SNAPSHOT_STATEMENT_BUF = (
        (SUBWAY_RESERVE_LINES * 64) +
        (SUBWAY_RESERVE_ALL_STATIONS * 64) +
        2 * 64
    );

    const std::unordered_map<std::string, std::string> SHUTTLE_NAMES{ //short id (valid url), name
        {"H", "SR"},  // Franklin Av
        {"FS", "SF"}, // Rockaway Park
        {"GS", "S"},  // 42nd St
        {"SI", "SIR"} // Staten Island Railway (not a shuttle)
    };

    //==========================================================================
}

// ===== COMMON FUNCTIONS ======================================================

namespace common {
    enum {
        NORMAL,
        SQLITE,
        MINSEC,
        DAY_TIME
    };

    const bool enablePanic = true;

    // takes a time and returns it in a string (no whitespace) format
    std::string formatTime(const time_t* time = nullptr, int mode = NORMAL);
    int panic(
        const std::string& misc, 
        const std::string& filename = __builtin_FILE(), 
        const std::string& funcname = __builtin_FUNCTION(),
        unsigned int line = __builtin_LINE());
}

// ===== CURL FUNCTIONS=========================================================

//for libcurl
namespace get_page {
    const unsigned int CONNECTION_TIMEOUT_WAIT = 5;

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