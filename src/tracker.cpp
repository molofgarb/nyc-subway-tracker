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
    std::vector<const std::string> data{
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