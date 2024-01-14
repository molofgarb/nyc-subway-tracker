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
    const std::vector<std::pair<std::string, std::string>> SNAPSHOT_COLUMNS{ // ex) Subway Stations

        std::make_pair("Subway_Snapshot", "TEXT"), // ex) Subway_1673908573

        std::make_pair("Type", "TEXT"), // ex) Subway

        std::make_pair("Time", "INTEGER"), // ex) 2024-01-06-15:19:25
        std::make_pair("Time_Unix", "INTEGER"), // ex) 1673908573
    };

    const Table SNAPSHOT_TABLE(SNAPSHOT_TABLE_NAME, SNAPSHOT_COLUMNS);

    // name is something like 
    const std::vector<std::pair<std::string, std::string>> SUBWAY_COLUMNS{
        std::make_pair("Table_Name", "TEXT"),
        std::make_pair("Type", "TEXT"),
        std::make_pair("Name", "TEXT")
    };

    // ex) Subway_Stations_1673908573, subway_station_snapshot
    const std::vector<std::pair<std::string, std::string>> LINES_COLUMNS{ 
        std::make_pair("StopID_and_Time", "TEXT"), // ex) Station_G14_1673908573
        std::make_pair("StopID", "TEXT"), // ex) G14
        std::make_pair("Name", "TEXT"), // ex) Roosevelt Av

        std::make_pair("Time_Checked", "INTEGER"), // ex) 2024-01-06-15:19:25
        std::make_pair("Time_Checked_Unix", "INTEGER") // ex) 1673908573
    };

    // ex) Station_G14_1673908573, station_snapshot
    const std::vector<std::pair<std::string, std::string>> STATION_COLUMNS{ 
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

    // ===== FUNCTIONS =========================================================

    // gets current system status and stores it in snapshots table
    int snapshot(const Subway& subway, const std::string& db_name); 

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