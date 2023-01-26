#include <iostream>
#include <fstream>

#include <string>
#include <utility>

#include <ctime>

#include "sqlite/sqlite3.h"

#include "tracker_sqlite.h"
#include "subway.h"
#include "line.h"
#include "station.h"

#include "tracker.h"


namespace tracker {

//initializes the database files used to keep track of snapshots
sqlite3* subway_db_initialize(sqlite3* db) {
    db = sqlite::open_db(db, constant::DB_NAME);

    db = sqlite::create_new_table(db, constant::SNAPSHOT_TABLE);

    return db;
}

//takes a snapshot -- adds current system status table into db snapshot
sqlite3* snapshot(const Subway& subway, sqlite3* db) {
    std::vector<std::string> data{
        tracker::subway_snapshot(subway, db),
        std::to_string((int)std::time(nullptr))
    }; //reference to current snapshot to be added

    sqlite::insert_row(db, constant::SNAPSHOT_TABLE, data);
    return db;
} 

//logs current subway system status as a table with station snapshots as contents
const std::string subway_snapshot(const Subway& subway, sqlite3* db) {
    if (db == nullptr) { //db doesnt exist
        std::cerr << "<error> no db at subway_snapshot" << std::endl;
    }

    //make table for subway system
    time_t time = std::time(nullptr); //time right now
    const std::string tableName = "Subway_" + std::to_string((int)time);
    Table table(
        tableName,
        constant::SUBWAY_STATIONS_COLUMNS
    ); //table containing every station in snapshot
    db = sqlite::create_new_table(db, table);

    //add entries for each station
    for (const auto stationPair : *subway.getStations()) { 
        //updates station, and then uses time of station update in primary key
        std::vector<std::string> data{
            tracker::station_snapshot(*stationPair.second, db), 
            stationPair.first,
            stationPair.second->getName()
        };

        sqlite::insert_row(db, table, data); //add new station table into subway snapshot
    }
    return tableName;
}

//logs current station status as a table with nearby arrivals as contents
const std::string station_snapshot(const Station& station, sqlite3* db) {
    if (db == nullptr) {
        std::cerr << "<error> no db at station_snapshot" << std::endl;
    }

    //make table for station
    time_t time = std::time(nullptr);
    const std::string tableName = "Station_" + station.getID() + "_" +
        std::to_string((int)station.getTime());
    Table table = Table(
        tableName,
        constant::STATION_COLUMNS
    );
    db = sqlite::create_new_table(db, table);

    //add entries for each nearby arrival
    for (const auto arrival : station.getNearby()) {
        std::string name = (*arrival.train).name;
        std::string time = std::to_string(arrival.time);

        //arrival time conversion and formatting
        std::time_t untilArrivalTime = arrival.time - std::time(nullptr); //diff arrival to now time
        std::time_t arrivalTime(int(arrival.time) % (86400)); //updateTime in sec from start of day
        char untilArrivalTimeStr[128] = "";
        char arrivalTimeStr[128] = "";
        std::strftime(untilArrivalTimeStr, sizeof(untilArrivalTimeStr), "%M:%S", std::localtime(&untilArrivalTime));
        std::strftime(arrivalTimeStr, sizeof(arrivalTimeStr), "%I:%M:%S %p", std::localtime(&arrivalTime));

        std::vector<std::string> data{
            name + "_" + time, 
            name,
            std::string(untilArrivalTimeStr),
            std::string(arrivalTimeStr),
            time,
            std::to_string((*arrival.train).dirID), 
        };

        sqlite::insert_row(db, table, data);
    }

    return tableName;
}

}