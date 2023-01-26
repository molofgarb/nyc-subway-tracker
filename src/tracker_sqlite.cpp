#include <iostream>
#include <fstream>

#include <vector>
#include <string>
#include <utility>

#include <ctime>

#include "sqlite/sqlite3.h"

#include "tracker_sqlite.h"


namespace sqlite {

sqlite3* open_db(sqlite3* db, const std::string& db_name) {
    int exit = sqlite3_open(db_name.data(), &db);
    if (exit) { // IMPORTANT <debug> error probably this
        std::cerr << "<error sql_helper::open_db()>" << std::endl;
        return nullptr;
    }
    return db;
}

sqlite3* create_new_table(sqlite3* db, const Table& table) {
    std::string zSql = "CREATE TABLE " +
                      table.name + "(\n";
    for (auto i = 0; i < (table.columns).size(); i++) { //name, data type
        auto name = (table.columns)[i].first;
        auto type = (table.columns)[i].second;
        zSql.append(
            name + ' ' + type + 
            ((i == 0) ? " PRIMARY KEY" : "") + 
            ((i != (table.columns).size() - 1) ? ",\n" : ");")
        );
    }

    std::cerr << "<debug> zSql: \n" << zSql << std::endl;
    sqlite3_stmt* createTable;
    int res0 = sqlite3_prepare_v2(db, zSql.data(), zSql.length(), &createTable, nullptr);
    int res1 = sqlite3_step(createTable);
    int res2 = sqlite3_finalize(createTable);
    if (res0 + res1 + res2 != 0) {
        std::cerr << "<error create_new_table> Table: " << table.name << ' ' <<
            res0 << ' ' << res1 <<  ' ' << res2 << std::endl;
    }
    return db;
}

sqlite3* insert_row(sqlite3* db, const Table& table, const std::vector<const std::string>& data) {
    std::string zSql = "INSERT INTO " + table.name + "(";

    for (auto i = 0; i < (table.columns).size(); i++) { //add table info
        zSql.append((table.columns)[i].first); //names of rows
        if (i != (table.columns).size() - 1) zSql.append(",");
    }
    zSql.append(")\nVALUES (");

    for (auto i = 0; i < data.size(); i++) { //add data info
        zSql.append("'" + data[i] + "'");
        if (i != data.size() - 1) zSql.append(",");
    }
    zSql.append(");\n");

    // std::cerr << "<debug> zSql: \n" << zSql << std::endl;
    sqlite3_stmt* insertData;
    int res0 = sqlite3_prepare_v2(db, zSql.data(), zSql.length(), &insertData, nullptr);
    int res1 = sqlite3_step(insertData);
    int res2 = sqlite3_finalize(insertData);
    if (res0 + res1 + res2 != 0) {
        std::cerr << "<error insert_row> Table: " << table.name << ' ' <<
            res0 << ' ' << res1 <<  ' ' << res2 << std::endl;
    }
    return db;
}

sqlite3* delete_row(sqlite3* db, const Table& table, const std::vector<const std::string>& data) {
    std::string primaryKey = table.columns[0].first;
    std::string zSql = "DELETE FROM " + table.name + "WHERE " +
        primaryKey + "=" + data[0] + ";"; //assuming primary key is always first column
    
    // std::cerr << "<debug> zSql: " << zSql << std::endl;
    sqlite3_stmt* deleteData;
    int res0 = sqlite3_prepare_v2(db, zSql.data(), zSql.length(), &deleteData, nullptr);
    int res1 = sqlite3_step(deleteData);
    int res2 = sqlite3_finalize(deleteData);
    // if (res0 + res1 + res2 != 0) {
    //     std::cerr << "<error delete_row> Table: " << table.name << ' ' <<
    //         res0 << ' ' << res1 <<  ' ' << res2 << std::endl;
    // }
    return db;
}

sqlite3* get_row(sqlite3* db, const Table& table, const std::vector<const std::string>& data) {
    std::string primaryKey = table.columns[0].first;
    std::string zSql = "SELECT * from " + table.name + " WHERE " +
        primaryKey + " = " + data[0] + ";";
    
    // std::cerr << "<debug> zSql: " << zSql << std::endl;
    sqlite3_stmt* getData;
    int res0 = sqlite3_prepare_v2(db, zSql.data(), zSql.length(), &getData, nullptr);
    int res1 = sqlite3_step(getData);
    int res2 = sqlite3_finalize(getData);
    // if (res0 + res1 + res2 != 0) {
    //     std::cerr << "<error get_row> Table: " << table.name << ' ' <<
    //         res0 << ' ' << res1 <<  ' ' << res2 << std::endl;
    // }
    return db;
}

}