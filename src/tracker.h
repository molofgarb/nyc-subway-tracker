#ifndef SQL_TRACKER_H
#define SQL_TRACKER_H

#include <iostream>

#include <string>
#include <utility>

#include <sqlite3.h>

#include "subway.h"
#include "line.h"
#include "station.h"



//pointerize everything because copying vectors is complex

struct Table {
    Table(const std::string& name, 
          const std::vector<std::pair<std::string, std::string>>* columns):
        name(name), columns(columns) {}

    const std::string name;
    const std::vector<std::pair<std::string, std::string>>* columns; //name, data type
};

struct Row {
    Row(const std::vector<std::pair<std::string, std::string>>* columns,
        const std::vector<std::string>* data): columns(columns), data(data) {}

    const std::vector<std::pair<std::string, std::string>>* columns; //name, data type
    const std::vector<std::string>* data;
};

namespace tracker {
    sqlite3* subway_initialize(const Subway& subway, sqlite3* db = nullptr);
    sqlite3* subway_update(Subway& subway);
    sqlite3* subway_output(const Subway& subway);

    sqlite3* line_initialize(const Line& line, sqlite3* db = nullptr);
    sqlite3* line_update(Line& line);
    sqlite3* line_output(const Line& line);

    sqlite3* station_initialize(const Station& station, sqlite3* db = nullptr);
    sqlite3* station_update(Station& station);
    sqlite3* station_output(const Station& station);
}

namespace sqlite {
    sqlite3* create_db(const std::string& db_name);
    sqlite3* open_db(const std::string& db_name);

    sqlite3* create_new_table(sqlite3* db, const Table& table);

    sqlite3* insert_row(sqlite3* db, const Table& table, const Row& row);
    sqlite3* delete_row(sqlite3* db, const Table& table, const Row& row);
    sqlite3* get_row(sqlite3* db, const Table& table, const Row& row);
}


#endif