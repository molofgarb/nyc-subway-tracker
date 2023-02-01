#ifndef TRACKER_H
#define TRACKER_H

#include <iostream>

#include <string>
#include <utility>

// external includes
#include <sqlite3.h>

// nyc-subway-tracker includes
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
        std::make_pair("Subway_Snapshot", "TEXT"), // ex) Subway_1673908573
        std::make_pair("Time", "INTEGER"), // ex) 1673908573
    };
    const Table SNAPSHOT_TABLE(
        SNAPSHOT_TABLE_NAME,
        SNAPSHOT_COLUMNS
    );

    const std::vector<std::pair<std::string, std::string>> SUBWAY_STATIONS_COLUMNS = { // ex) Subway_1673908573
        std::make_pair("StopID_and_Time", "TEXT"), // ex) G14_1673908573
        std::make_pair("StopID", "TEXT"), // ex) G14
        std::make_pair("Name", "TEXT") // ex) Roosevelt Av
    };

    const std::vector<std::pair<std::string, std::string>> STATION_COLUMNS = { // ex) Station_G14_1673908573
        std::make_pair("Name_and_Time", "TEXT"), // ex) F_1673909000
        std::make_pair("Name", "TEXT"), // ex) F
        std::make_pair("Time_Until_Arrival", "TEXT"), //ex) 05:33
        std::make_pair("Time_of_Arrival", "TEXT"), // ex) 12:28:00;
        std::make_pair("Time_of_Arrival_Unix", "INTEGER"), // ex) 1673909000
        std::make_pair("Direction_ID", "INTEGER") // ex) 1
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
    const std::string subway_snapshot(const Subway& subway, sqlite3* db);

    //gets current line status as a bunch of station snapshots
    const std::string line_snapshot(const Line& line, sqlite3* db); //WIP

    //gets current station snapshot
    const std::string station_snapshot(const Station& station, sqlite3* db);

    //WIP
    sqlite3* subway_output(const Subway& subway, sqlite3* db);
    sqlite3* line_output(const Line& line, sqlite3* db);
    sqlite3* station_output(const Station& station, sqlite3* db);
}

#endif