#include <iostream>
#include <fstream>

#include <vector>
#include <string>
#include <utility>

#include <ctime>

#include <sqlite3.h>

#include "tracker_sqlite.h"


namespace sqlite {

sqlite3* open_db(sqlite3* db, const std::string& db_name) {
    char* error;

    int exit = sqlite3_open(db_name.data(), &db);
    if (exit) { // IMPORTANT <debug> error probably this
        std::cerr << "<error sql_helper::open_db()>" << std::endl;
        return nullptr;
    }
    return db;
}

sqlite3* create_new_table(sqlite3* db, const Table& table) {
    char* error = nullptr;
    std::string zSql = "CREATE TABLE " +
                      table.name + "(\n";
    for (auto i = 0; i < (table.columns).size(); i++) { //name, data type
        auto name = (table.columns)[i].first;
        auto type = (table.columns)[i].second;
        zSql.append(
            name + ' ' + type + ' ' + 
            ((i == 0) ? "PRIMARY KEY " : "") + 
            ((i != (table.columns).size() - 1) ? ",\n" : ");"));

    }

    std::cerr << "<debug> zSql: " << zSql << std::endl;
    sqlite3_stmt* createTable;
    sqlite3_prepare_v2(db, zSql.data(), zSql.length(), &createTable, nullptr);
    sqlite3_step(createTable);
    sqlite3_finalize(createTable);
    return db;
}

sqlite3* insert_row(sqlite3* db, const Table& table, const std::vector<const std::string>& data) {
    std::string zSql = "INSERT INTO " + table.name + " (";

    for (auto i = 0; i < (table.columns).size(); i++) { //add table info
        zSql.append((table.columns)[i].second); //names of rows
        if (i != (table.columns).size() - 1) zSql.append(",");
    }
    zSql.append(")\nVALLUES (");

    for (auto i = 0; i < data.size(); i++) { //add data info
        zSql.append(data[i]);
        if (i != data.size() - 1) zSql.append(",");
    }
    zSql.append(");\n");

    std::cerr << "<debug> zSql: " << zSql << std::endl;
    sqlite3_stmt* insertData;
    sqlite3_prepare_v2(db, zSql.data(), zSql.length(), &insertData, nullptr);
    sqlite3_step(insertData);
    sqlite3_finalize(insertData);
    return db;
}

sqlite3* delete_row(sqlite3* db, const Table& table, const std::vector<const std::string>& data) {
    std::string primaryKey = table.columns[0].first;
    std::string zSql = "DELETE FROM " + table.name + "WHERE " +
        primaryKey + "=" + data[0] + ";"; //assuming primary key is always first column
    
    std::cerr << "<debug> zSql: " << zSql << std::endl;
    sqlite3_stmt* deleteData;
    sqlite3_prepare_v2(db, zSql.data(), zSql.length(), &deleteData, nullptr);
    sqlite3_step(deleteData);
    sqlite3_finalize(deleteData);
    return db;
}

sqlite3* get_row(sqlite3* db, const Table& table, const std::vector<const std::string>& data) {
    std::string primaryKey = table.columns[0].first;
    std::string zSql = "SELECT * from " + table.name + " WHERE " +
        primaryKey + " = " + data[0] + ";";
    
    std::cerr << "<debug> zSql: " << zSql << std::endl;
    sqlite3_stmt* getData;
    sqlite3_prepare_v2(db, zSql.data(), zSql.length(), &getData, nullptr);
    sqlite3_step(getData);
    sqlite3_finalize(getData);
    return db;
}

}