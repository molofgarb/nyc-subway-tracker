#ifndef SQL_TRACKER_H
#define SQL_TRACKER_H

#include <iostream>

#include <string>
#include <utility>

#include <sqlite3.h>

#include "subway.h"
#include "line.h"
#include "station.h"

const std::string DB_NAME = "nyc-subway-tracker.db";

//pointerize everything because copying vectors is complex

struct Table {
    Table(const std::string& name, 
          const std::vector<std::pair<std::string, std::string>>& columns):
        name(name), columns(columns) {}

    std::string name;
    std::vector<std::pair<std::string, std::string>> columns; //name, data type
};

struct Row {
    Row(const std::vector<std::string>& types,
        const std::vector<std::string>& data):
        types(types), data(data) {}

    std::vector<std::string> types;
    std::vector<std::string> data; 
};

namespace tracker {
    void subway_initialize();
    void subway_update();

    void line_initialize();
    void line_update();

    void station_initialize();
    void station_update();
}

namespace sql {
    sqlite3* create_db(const std::string& db_name);
    sqlite3* open_db(const std::string& db_name);

    sqlite3* create_table(sqlite3* db, const Table& table);
    sqlite3* copy_table(sqlite3* db, const Table& oldTable, const Table& newTable);

    sqlite3* insert_row(sqlite3* db, const Table& table, const std::vector<std::string> row);
    sqlite3* delete_row(sqlite3* db, const Table& table, const std::vector<std::string> row);
    sqlite3* get_row(sqlite3* db, const Table& table, const std::vector<std::string> row);
}


#endif