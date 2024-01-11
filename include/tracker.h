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

namespace tracker {
    const bool SNAPSHOT_STATIONS = false;
    
    //  initializes snapshot
    int snapshotDBInitialize(SqliteEnv& env); 

    // gets current system status and stores it in snapshots table
    int snapshot(const Subway& subway); 

    // Helpers //
    // they typically return the name of the table that they made

    // gets current subway status in terms of station snapshot s
    std::string subwaySnapshot(const Subway& subway, const SqliteEnv& env);
    int subwaySnapshotThread(
        const std::unordered_map<std::string, st_ptr>& all_stations, 
        const SqliteEnv& env, 
        const Table& table, 
        size_t offset,
        size_t num_threads);

    //gets current line snapshot
    std::string lineSnapshot(const Line& line, const SqliteEnv& env);

    //gets current stations snapshot
    std::string stationSnapshot(const Station& station, const SqliteEnv& env);

    //WIP
    sqlite3* subway_output(const Subway& subway, sqlite3* db);
    sqlite3* line_output(const Line& line, sqlite3* db);
    sqlite3* station_output(const Station& station, sqlite3* db);
}

#endif