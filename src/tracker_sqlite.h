#ifndef TRACKER_SQLITE_H
#define TRACKER_SQLITE_H

#include <iostream>

#include <vector>
#include <string>
#include <utility>

#include <ctime>

#include <sqlite3.h>

struct Table {
    Table(const std::string& name, 
          const std::vector<std::pair<std::string, std::string>> columns):
        name(name), columns(columns) {}

    const std::string name;
    const std::vector<std::pair<std::string, std::string>> columns; //name, data type
};

namespace sqlite {
    sqlite3* create_db(const std::string& db_name);
    sqlite3* open_db(const std::string& db_name);

    sqlite3* create_new_table(sqlite3* db, const Table& table);

    sqlite3* insert_row(sqlite3* db, const Table& table, const std::vector<const std::string> data);
    sqlite3* delete_row(sqlite3* db, const Table& table, const std::vector<const std::string> data);
    sqlite3* get_row(sqlite3* db, const Table& table, const std::vector<const std::string> data);
}

#endif