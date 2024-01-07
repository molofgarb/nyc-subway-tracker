#ifndef TRACKER_H
#define TRACKER_H

// external includes
#include <sqlite3.h>

#include <common.h>

// nyc-subway-tracker includes
#include <subway.h>
#include <line.h>
#include <station.h>

#include <tracker_sqlite.h>

//pointerize everything because copying vectors is complex
//make templated version later maybe

namespace constant {
    // snapshot
    const std::string SNAPSHOT_TABLE_NAME = "Snapshots";
    const std::vector<std::pair<std::string, std::string>> SNAPSHOT_COLUMNS = { // ex) Subway Stations
        std::make_pair("Subway_Snapshot", "TEXT"), // ex) Subway_1673908573
        std::make_pair("Time", "INTEGER"), // ex) 1673908573
    };
    const Table SNAPSHOT_TABLE(
        SNAPSHOT_TABLE_NAME,
        SNAPSHOT_COLUMNS
    );

    //-------------------------------------------------------------------------

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
        std::make_pair("Time_Checked", "INTEGER") // ex) 1673908573
    };

    // ex) Station_G14_1673908573, station_snapshot
    const std::vector<std::pair<std::string, std::string>> STATION_COLUMNS = { 
        std::make_pair("Name_and_Time", "TEXT"), // ex) Nearby_F_1673909000
        std::make_pair("Name", "TEXT"), // ex) F
        std::make_pair("Direction_ID", "INTEGER"), // ex) 1
        std::make_pair("Headsign", "TEXT"), // ex) Manhattan
        std::make_pair("Time_Checked", "TEXT"),
        std::make_pair("Time_Until_Arrival", "TEXT"), //ex) 05:33
        std::make_pair("Time_of_Arrival", "TEXT"), // ex) 12:28:00;
    };

    //-------------------------------------------------------------------------

// For each snapshot:
// SNAPSHOTS (one)
//          |--> SUBWAY (one)
//                     |--> STATIONS (mul)
//                                  |--> STATION_DATA (data)
//                     |--> LINES (mul)
//                                  |--> STATIONS (mul) 
//                                               |--> STATION_DATA (data)
}

namespace tracker {
    
    sqlite3* snapshot_db_initialize(); //initializes snapshot -- creates the file
    sqlite3* snapshot(const Subway& subway, sqlite3* db); //gets current system status and stores it in snapshots table

    // Helpers //
    //gets current subway status in terms of station snapshot s
    const std::string subway_snapshot(const Subway& subway, sqlite3* db = nullptr, time_t time = 0);

    //gets current line snapshot
    const std::string line_snapshot(const Line& line, sqlite3* db = nullptr, time_t time = 0);

    //gets current stations snapshot
    const std::string station_snapshot(const Station& station, sqlite3* db = nullptr, time_t time = 0);

    //WIP
    sqlite3* subway_output(const Subway& subway, sqlite3* db);
    sqlite3* line_output(const Line& line, sqlite3* db);
    sqlite3* station_output(const Station& station, sqlite3* db);
}

#endif