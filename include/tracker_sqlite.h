#ifndef TRACKER_SQLITE_H
#define TRACKER_SQLITE_H

#include <iostream>

#include <vector>
#include <string>
#include <utility>

#include <ctime>

// external includes
#include <sqlite3.h>


struct Table {
    Table(const std::string& name, 
          const std::vector<std::pair<std::string, std::string>> columns):
        name(name), columns(columns) {}

    const std::string name;
    const std::vector<std::pair<std::string, std::string>> columns; //name, data type
};


namespace sqlite {
    sqlite3* open_db(sqlite3* db, const std::string& db_name); //create/open

    sqlite3* create_new_table(sqlite3* db, const Table& table);

    sqlite3* insert_row(sqlite3* db, const Table& table, const std::vector<std::string>& data);
    sqlite3* delete_row(sqlite3* db, const Table& table, const std::string& data);
    sqlite3* get_row(sqlite3* db, const Table& table, const std::string& data);
}

#endif