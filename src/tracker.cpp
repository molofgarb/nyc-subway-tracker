#include <iostream>
#include <fstream>

#include <string>
#include <utility>

#include <sqlite3.h>

#include "subway.h"
#include "line.h"
#include "station.h"

#include "tracker.h"

namespace tracker {

}

namespace sql {

sqlite3* create_db(const std::string& db_name) {
    std::ifstream file(db_name);
    return open_db(db_name);
}

sqlite3* open_db(const std::string& db_name) {
    sqlite3* db;
    char* error;

    int exit = sqlite3_open(db_name.data(), &db);
    if (exit) {
        std::cerr << "<error sql_helper::open_db()>" << std::endl;
        return nullptr;
    }
    return db;
}

sqlite3* create_new_table(sqlite3* db, const Table& table) {
    char* error = nullptr;
    std::string zSql = "CREATE TABLE " +
                      table.name + "(\n";
    for (auto i = 0; i < table.columns.size(); i++) { //name, data type
        auto name = table.columns[i].first;
        auto type = table.columns[i].second;
        zSql.append(
            name + ' ' + type + ' ' + 
            ((i == 0) ? "PRIMARY KEY " : "") + 
            ((i != table.columns.size() - 1) ? ",\n" : ");"));

    }

    sqlite3_stmt* createTable;
    sqlite3_prepare_v2(db, zSql.data(), zSql.length(), &createTable, nullptr);
    sqlite3_step(createTable);
    sqlite3_finalize(createTable);
    return db;
}

sqlite3* insert_data(sqlite3* db, const Table& table, const std::vector<std::string> row) {
    std::string zSql = "INSERT INTO " + table.name + " (";
    for (auto i = 0; i < table.columns.size(); i++) { //list table col names
        zSql.append(table.columns[i].first + //names of rows
            ((i != row.size() - 1) ? "," : ")\n")); 
    }
    zSql.append("VALUES (");
    for (auto i = 0; i < row.size(); i++) { //add row
        zSql.append(row[i] + //data in row
            ((i != row.size() - 1) ? "," : ");\n"));
    }
    zSql.append("\n");

}

}