#ifndef TRACKER_H
#define TRACKER_H

#include <pch.h>

// external includes
#include <sqlite3.h>

// nyc-subway-tracker includes
#include <subway.h>
#include <line.h>
#include <station.h>

#include <tracker_sqlite.h>

namespace tracker {
    
    // ===== CONSTANTS =========================================================

    const std::string SNAPSHOT_TABLE_NAME = "Snapshots";
    const std::vector<std::string> SNAPSHOT_COLUMNS{ // ex) Subway Stations

        "Subway_Snapshot",// ex) Subway_1673908573

        "Type", // ex) Subway

        "Time", // ex) 2024-01-06-15:19:25
        "Time_Unix", // ex) 1673908573
    };

    const Table SNAPSHOT_TABLE(SNAPSHOT_TABLE_NAME, SNAPSHOT_COLUMNS);

    // name is something like 
    const std::vector<std::string> SUBWAY_COLUMNS{
        "Table_Name", 
        "Type", 
        "Name", 
    };

    // ex) Subway_Stations_1673908573, subway_station_snapshot
    const std::vector<std::string> LINES_COLUMNS{ 
        "StopID_and_Time", // ex) Station_G14_1673908573
        "StopID", // ex) G14
        "Name", // ex) Roosevelt Av

        "Time_Checked", // ex) 2024-01-06-15:19:25
        "Time_Checked_Unix", // ex) 1673908573
    };

    // ex) Station_G14_1673908573, station_snapshot
    const std::vector<std::string> STATION_COLUMNS{ 
        "Name_and_DirID_and_Time", // ex) Nearby_F_1673909000

        "Name", // ex) F
        "Direction_ID", // ex) 1
        "Headsign", // ex) Manhattan

        "Time_Checked", // ex) 2024-01-06-15:19:25
        "Time_Until_Arrival", //ex) 05:33
        "Time_of_Arrival", // ex) 2024-01-06-15:19:25

        "Time_Checked_Unix", 
        "Time_Until_Arrival_Unix", //ex) 05:33
        "Time_of_Arrival_Unix", // ex) 2024-01-06-15:19:25
    };

    // ===== FUNCTIONS =========================================================

    // gets current system status and stores it in snapshots table
    int snapshot(const Subway& subway, const std::string& db_name); 

    int debugAllRows(const std::string& db_name);

    // Helpers //
    // they typically return the name of the table that they made

    // gets current subway status in terms of station snapshot s
    std::string subwaySnapshot(TSqlite& db, const Subway& subway);
    int subwaySnapshotThread(
        TSqlite* db, 
        const std::unordered_map<std::string, st_ptr>& all_stations, 
        const Table& table, 
        size_t offset,
        size_t num_threads);

    //gets current line snapshot
    std::string lineSnapshot(
        TSqlite& db, 
        const Line& line, 
        bool SNAPSHOT_STATIONS = false);

    //gets current stations snapshot
    std::string stationSnapshot(TSqlite& db, const Station& station);

    //WIP
    sqlite3* subway_output(const Subway& subway, sqlite3* db);
    sqlite3* line_output(const Line& line, sqlite3* db);
    sqlite3* station_output(const Station& station, sqlite3* db);
}

#endif