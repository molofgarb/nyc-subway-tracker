#include <iostream>
#include <fstream>

#include <string>
#include <utility>

#include <ctime>

#include <sqlite3.h>

#include "subway.h"
#include "line.h"
#include "station.h"
#include "common.h"

#include "tracker.h"


namespace tracker {

sqlite3* subway_db_initialize() {
    sqlite3* db = sqlite::create_db("subway.db");

    Table subwayStations("Subway Stations", constant::SNAPSHOT_COLUMNS);
    db = sqlite::create_new_table(db, subwayStations);

    return db;
}

sqlite3* snapshot(const Subway& subway, sqlite3* db) {
    time_t time = tracker::subway_snapshot(subway, db);

    Table subwayStations(
        "Subway Snapshots", 
        constant::SNAPSHOT_COLUMNS
    );
    std::vector<std::string> data{
        "Stations " + time
    }; 

    sqlite::insert_row(db, subwayStations, data);
    return db;
} 

time_t subway_snapshot(const Subway& subway, sqlite3* db /*= nullptr*/) {
    if (db == nullptr) {
        std::cerr << "<error> no db" << std::endl;
    }

    //make table for subway system
    time_t time = std::time(nullptr);
    Table table(
        "Stations " + std::to_string((int)time),
        constant::SUBWAY_STATIONS_COLUMNS
    );
    db = sqlite::create_new_table(db, table);

    //add entries for each station
    for (const auto stationPair : *subway.getStations()) {
        std::vector<const std::string> data{
            //updates station, and then uses time of station update in primary key
            stationPair.first + std::to_string((int)tracker::station_snapshot(*stationPair.second)), 
            stationPair.first,
            stationPair.second->getName()
        };
        sqlite::insert_row(db, table, data);
    }
}

time_t station_snapshot(const Station& station, sqlite3* db /*= nullptr*/) {
    if (db == nullptr) {
        std::cerr << "<error> no db" << std::endl;
    }

    //make table for station
    time_t time = std::time(nullptr);
    Table table = Table(station.getID() + " " + std::to_string((int)std::time(nullptr)),
        constant::STATION_COLUMNS);
    db = sqlite::create_new_table(db, table);

    //add entries for each nearby arrival
    for (const auto arrival : station.getNearby()) {
        std::vector<const std::string> data{
            (*arrival.train).name, 
            std::to_string((*arrival.train).dirID), 
            std::to_string(arrival.time)
        };
        sqlite::insert_row(db, table, data);
    }
    return time;
}



}

namespace sqlite {

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

sqlite3* insert_row(sqlite3* db, const Table& table, const std::vector<std::string> data) {
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

sqlite3* delete_row(sqlite3* db, const Table& table, const std::vector<std::string> data) {
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

sqlite3* get_row(sqlite3* db, const Table& table, const std::vector<std::string> data) {
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