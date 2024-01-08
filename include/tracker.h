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
    
    sqlite3* snapshot_db_initialize(); //initializes snapshot -- creates the file
    sqlite3* snapshot(const Subway& subway, sqlite3* db); //gets current system status and stores it in snapshots table

    // Helpers //
    // they typically return the name of the table that they made

    //gets current subway status in terms of station snapshot s
    const std::string subway_snapshot(const Subway& subway, sqlite3* db = nullptr, time_t time = 0);

    //gets current line snapshot
    const std::string line_snapshot(const Line& line, sqlite3* db = nullptr, time_t time = 0, bool snapshot_stations = false);

    //gets current stations snapshot
    const std::string station_snapshot(const Station& station, sqlite3* db = nullptr, time_t time = 0);

    //WIP
    sqlite3* subway_output(const Subway& subway, sqlite3* db);
    sqlite3* line_output(const Line& line, sqlite3* db);
    sqlite3* station_output(const Station& station, sqlite3* db);
}

#endif