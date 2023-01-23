#include <iostream>
#include <fstream>

#include <string>
#include <utility>

#include <ctime>

#include <sqlite3.h>

#include "tracker_sqlite.h"
#include "subway.h"
#include "line.h"
#include "station.h"
#include "common.h"

#include "tracker.h"


namespace tracker {

//initializes the database files used to keep track of snapshots
sqlite3* subway_db_initialize(sqlite3* db) {
    std::cerr << "before open, the db is at " << db << std::endl;
    db = sqlite::open_db(db, constant::DB_NAME);
    std::cerr << "after open, the db is at " << db << std::endl;

    Table subwayStations("Subway Stations", constant::SNAPSHOT_COLUMNS);
    db = sqlite::create_new_table(db, subwayStations);

    return db;
}

//takes a snapshot -- adds current system status table into db snapshot
sqlite3* snapshot(const Subway& subway, sqlite3* db) {
    time_t time = tracker::subway_snapshot(subway, db); //time of snapshot

    Table subwayStations(
        "Subway Snapshots", 
        constant::SNAPSHOT_COLUMNS
    ); //snapshot table
    std::vector<const std::string> data{
        "Stations " + std::to_string((int)time)
    }; //reference to current snapshot to be added

    sqlite::insert_row(db, subwayStations, data);
    return db;
} 

//logs current subway system status in a new table
time_t subway_snapshot(const Subway& subway, sqlite3* db) {
    if (db == nullptr) { //db doesnt exist
        std::cerr << "<error> no db at subway_snapshot" << std::endl;
    }

    //make table for subway system
    time_t time = std::time(nullptr); //time right now
    Table table(
        "Stations " + std::to_string((int)time),
        constant::SUBWAY_STATIONS_COLUMNS
    ); //table containing every station in snapshot
    db = sqlite::create_new_table(db, table);

    //add entries for each station
    for (const auto stationPair : *subway.getStations()) { //pair.first is stopID
        std::vector<const std::string> data{
            //updates station, and then uses time of station update in primary key
            stationPair.first + std::to_string((int)tracker::station_snapshot(*stationPair.second, db)), 
            stationPair.first,
            stationPair.second->getName()
        };
        sqlite::insert_row(db, table, data); //add new station table into subway snapshot
    }
    return time;
}

time_t station_snapshot(const Station& station, sqlite3* db) {
    if (db == nullptr) {
        std::cerr << "<error> no db at station_snapshot" << std::endl;
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