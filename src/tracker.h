#ifndef SQL_TRACKER_H
#define SQL_TRACKER_H

#include <iostream>

#include <string>
#include <utility>

#include <ctime>

#include <sqlite3.h>

#include "subway.h"
#include "line.h"
#include "station.h"



//pointerize everything because copying vectors is complex
//make templated version later maybe

struct Table {
    Table(const std::string& name, 
          const std::vector<std::pair<std::string, std::string>> columns):
        name(name), columns(columns) {}

    const std::string name;
    const std::vector<std::pair<std::string, std::string>> columns; //name, data type
};

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

namespace sqlite {
    sqlite3* create_db(const std::string& db_name);
    sqlite3* open_db(const std::string& db_name);

    sqlite3* create_new_table(sqlite3* db, const Table& table);

    sqlite3* insert_row(sqlite3* db, const Table& table, const std::vector<const std::string> data);
    sqlite3* delete_row(sqlite3* db, const Table& table, const std::vector<const std::string> data);
    sqlite3* get_row(sqlite3* db, const Table& table, const std::vector<const std::string> data);
}


#endif