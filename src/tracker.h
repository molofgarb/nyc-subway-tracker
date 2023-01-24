#ifndef TRACKER_H
#define TRACKER_H

#include <iostream>

#include <string>
#include <utility>

#include <sqlite3.h>

#include "subway.h"
#include "line.h"
#include "station.h"

#include "tracker_sqlite.h"

//pointerize everything because copying vectors is complex
//make templated version later maybe

namespace constant {
    const std::string DB_NAME = "nyc-subway-tracker.db";

    const std::string SNAPSHOT_TABLE_NAME = "Subway_Snapshots";
    const std::vector<std::pair<std::string, std::string>> SNAPSHOT_COLUMNS = { // ex) Subway Stations
        std::make_pair("Subway_Snapshot", "TINYTEXT"), // ex) Stations 1673908573
        std::make_pair("Time", "BIGINT(16)"), // ex) 1673908573
    };
    const Table SNAPSHOT_TABLE(
        SNAPSHOT_TABLE_NAME,
        SNAPSHOT_COLUMNS
    );

    const std::vector<std::pair<std::string, std::string>> SUBWAY_STATIONS_COLUMNS = { // ex) Stations_1673908573
        std::make_pair("StopID_and_Time", "TINYTEXT"), // ex) G14 1673908573
        std::make_pair("StopID", "TINYTEXT"), // ex) G14
        std::make_pair("Name", "TINYTEXT") // ex) Roosevelt Av
    };

    const std::vector<std::pair<std::string, std::string>> STATION_COLUMNS = { // ex) G14_1673908573
        std::make_pair("Name", "TINYTEXT"), // ex) F
        std::make_pair("DirID", "TINYTEXT"), // ex) 1
        std::make_pair("Time_of_Arrival", "TINYTEXT") // ex) 1673909000
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

namespace tracker {
    sqlite3* subway_db_initialize(sqlite3* db); //initializes snapshot -- creates the file
    sqlite3* snapshot(const Subway& subway, sqlite3* db); //gets current system status and stores it in snapshots table

    //these functions are used by snapshot() -- ordinarily should not be called

    //gets current subway status in terms of a large station snapshot and in terms of line snapshot
    time_t subway_snapshot(const Subway& subway, sqlite3* db);

    //gets current line status as a bunch of station snapshots
    time_t line_snapshot(const Line& line, sqlite3* db); //WIP

    //gets current station snapshot
    time_t station_snapshot(const Station& station, sqlite3* db);

    //WIP
    sqlite3* subway_output(const Subway& subway, sqlite3* db);
    sqlite3* line_output(const Line& line, sqlite3* db);
    sqlite3* station_output(const Station& station, sqlite3* db);
}

#endif