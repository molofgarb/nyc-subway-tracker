#ifndef TRACKER_H
#define TRACKER_H

#include <iostream>

#include <string>
#include <utility>

#include <sqlite3.h>

#include "tracker_sqlite.h"
#include "subway.h"
#include "line.h"
#include "station.h"


//pointerize everything because copying vectors is complex
//make templated version later maybe

namespace tracker {
    sqlite3* subway_db_initialize(); //initializes snapshot -- creates the file
    sqlite3* snapshot(const Subway& subway, sqlite3* db); //gets current system status and stores it in snapshots table

    time_t subway_snapshot(const Subway& subway, sqlite3* db = nullptr);
    time_t line_snapshot(const Line& line, sqlite3* db = nullptr);
    time_t station_snapshot(const Station& station, sqlite3* db = nullptr);

    sqlite3* subway_output(const Subway& subway, sqlite3* db = nullptr);
    sqlite3* line_output(const Line& line, sqlite3* db = nullptr);
    sqlite3* station_output(const Station& station, sqlite3* db = nullptr);
}

#endif