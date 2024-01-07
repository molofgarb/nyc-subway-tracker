#ifndef TRACKER_SQLITE_H
#define TRACKER_SQLITE_H

// external includes
#include <sqlite3.h>

#include <common.h>

struct Table {
    Table(const std::string& name, 
          const std::vector<std::pair<std::string, std::string>> columns):
        name(name), columns(columns) {}

    const std::string name;
    const std::vector<std::pair<std::string, std::string>> columns; //name, data type
};


namespace sqlite {
    sqlite3* open_db(const std::string& db_name); //create/open

    sqlite3* create_new_table(sqlite3* db, const Table& table);

    sqlite3* insert_row(sqlite3* db, const Table& table, const std::vector<std::string>& data);
    sqlite3* delete_row(sqlite3* db, const Table& table, const std::string& data);
    sqlite3* get_row(sqlite3* db, const Table& table, const std::string& data);

    void exec_zSql(
        sqlite3* db = nullptr, const std::string& zSql = "", const std::string& funcname = "", 
        int (*callback)(void*,int,char**,char**) = nullptr, void* cbarg = nullptr
    );
    // 2nd arg to callback is num of result columns, 3rd arg is an array of strings
    // about the type of each result, 4th arg is an array of results as strings
}

#endif