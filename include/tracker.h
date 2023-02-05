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

    //-------------------------------------------------------------------------

    // snapshot
    const std::string SNAPSHOT_TABLE_NAME = "Subway_Snapshots";
    const std::vector<std::pair<std::string, std::string>> SNAPSHOT_COLUMNS = { // ex) Subway Stations
        std::make_pair("Subway_Snapshot", "TEXT"), // ex) Subway_1673908573
        std::make_pair("Time", "INTEGER"), // ex) 1673908573
    };
    const Table SNAPSHOT_TABLE(
        SNAPSHOT_TABLE_NAME,
        SNAPSHOT_COLUMNS
    );

    //-------------------------------------------------------------------------

    // ex) Subway_Stations_1673908573, subway_station_snapshot
    const std::vector<std::pair<std::string, std::string>> SUBWAY_STATIONS_COLUMNS = { 
        std::make_pair("StopID_and_Time", "TEXT"), // ex) Station_G14_1673908573
        std::make_pair("StopID", "TEXT"), // ex) G14
        std::make_pair("Name", "TEXT"), // ex) Roosevelt Av
        std::make_pair("Time", "INTEGER") // ex) Roosevelt Av
    };

    // ex) Station_G14_1673908573, station_snapshot
    const std::vector<std::pair<std::string, std::string>> STATION_COLUMNS = { 
        std::make_pair("Name_and_Time", "TEXT"), // ex) F_1673909000
        std::make_pair("Name", "TEXT"), // ex) F
        std::make_pair("Time_Checked", "TEXT"),
        std::make_pair("Time_Until_Arrival", "TEXT"), //ex) 05:33
        std::make_pair("Time_of_Arrival", "TEXT"), // ex) 12:28:00;
        std::make_pair("Time_Checked_Unix", "TEXT"),
        std::make_pair("Time_of_Arrival_Unix", "INTEGER"), // ex) 1673909000
        std::make_pair("Direction_ID", "INTEGER") // ex) 1
    };

    //-------------------------------------------------------------------------

    // ex) Subway_Lines_1673908573, subway_line_snapshot
    const std::vector<std::pair<std::string, std::string>> SUBWAY_LINES_COLUMNS = { 
        std::make_pair("Line_and_Time", "TEXT"), // ex) F_1673908573
        std::make_pair("Line", "TEXT"), // ex) F
        std::make_pair("Time", "INTEGER") // ex) 1673908573
    };

    // ex) Subway_Lines_1673908573, line_snapshot
    const std::vector<std::pair<std::string, std::string>> LINES_COLUMNS = SUBWAY_STATIONS_COLUMNS; 

}

namespace tracker {
    sqlite3* subway_db_initialize(sqlite3* db); //initializes snapshot -- creates the file
    sqlite3* snapshot(const Subway& subway, sqlite3* db); //gets current system status and stores it in snapshots table

    //WIP
    sqlite3* subway_output(const Subway& subway, sqlite3* db);
    sqlite3* line_output(const Line& line, sqlite3* db);
    sqlite3* station_output(const Station& station, sqlite3* db);
}

#endif