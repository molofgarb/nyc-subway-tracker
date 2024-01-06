// external includes
#include <sqlite3.h>

#include <common.h>

// nyc-subway-tracker includes
#include <tracker_sqlite.h>
#include <subway.h>
#include <line.h>
#include <station.h>

#include <tracker.h>

#define FILENAME "tracker.cpp"

namespace tracker {

//initializes the database files used to keep track of snapshots
sqlite3* snapshot_db_initialize() {
    sqlite3* db = sqlite::open_db(constant::DB_NAME);
    sqlite::create_new_table(db, constant::SNAPSHOT_TABLE);

    return db;
}

//takes a snapshot -- adds current system status table into db snapshot
//Creates table Subway_Snapshots using constant::SNAPSHOT_TABLE
sqlite3* snapshot(const Subway& subway, sqlite3* db) {
    if (db == nullptr)
        common::panic(FILENAME, "snapshot", "no db");

    time_t time = std::time(nullptr);

    std::vector<std::string> stationData{
        tracker::subway_snapshot(subway, db, time),
        common::formatTime()
    };
    sqlite::insert_row(db, constant::SNAPSHOT_TABLE, stationData);

    return db;
} 

// ----------------------------------------------------------------------------

//logs current subway system status as a table with station snapshots as contents
//Creates tables like Subway_Stations_1673908573 using constant::SUBWAY_STATIONS_COLUMNS
const std::string subway_snapshot(const Subway& subway, sqlite3* db, time_t time) {
    if (db == nullptr || time == 0)
        common::panic(FILENAME, "subway_snapshot", "no db");

    //make table for subway table that holds stations
    const std::string tableName = "Subway_" + common::formatTime(&time, true);

    //table containing every station in snapshot
    Table table(
        tableName,
        constant::SUBWAY_COLUMNS
    ); 
    sqlite::create_new_table(db, table);

    // add an entry for each station
    for (const auto& stationpair : subway.getStations()) {
        std::vector<std::string> data{
            tracker::station_snapshot(*stationpair.second, db, time),
            "Station",
            (stationpair.second)->getName()
        };
        sqlite::insert_row(db, table, data);
    }

    // add an entry for each line
    for (const auto& line : subway.getLines()) {
        std::vector<std::string> data{
            tracker::line_snapshot(line, db, time),
            "Line",
            line.getName()
        };
        sqlite::insert_row(db, table, data);
    }

    return tableName;
}

const std::string line_snapshot(const Line& line, sqlite3* db, time_t time) {
    if (db == nullptr || time == 0)
        common::panic(FILENAME, "lines_snapshot", "no db");

    // make table for line
    const std::string tableName = "Line_" + line.getName()  + "_" + common::formatTime(&time, true);
    Table table = Table(
        tableName,
        constant::LINES_COLUMNS
    );
    sqlite::create_new_table(db, table);

    // add entries for each station in line
    // station update NOT performed -- subway_snapshot will do it
    for (const auto& station : line.getStations()) {
        std::vector<std::string> data{
            "Station_" + station->getID() + "_" + common::formatTime(&time),
            station->getID(),
            station->getName(),
            station->getfTime()
        };
        sqlite::insert_row(db, table, data); // add new station table into subway snapshot
    }

    return tableName;
}

// logs current station status as a table with nearby arrivals as contents
// Creates tables like Station_G14_1673908573 using constant::STATION_COLUMNS
const std::string station_snapshot(const Station& station, sqlite3* db, time_t time) {
    if (db == nullptr || time == 0)
        common::panic(FILENAME, "subway_station_snapshot", "no db");

    //make table for station
    const std::string tableName = "Station_" + station.getID() + "_" + common::formatTime(&time, true);
    Table table = Table(
        tableName,
        constant::STATION_COLUMNS
    );
    sqlite::create_new_table(db, table);

    //add entries for each nearby arrival
    size_t i = 0;
    for (const auto& arrival : station.getNearby()) {
        time_t timeUntilArrival = arrival.time - station.getTime();
        std::string strtimeUntilArrival = 
            std::to_string(timeUntilArrival / 60) + ":" + std::to_string(timeUntilArrival % 60);

        std::vector<std::string> data{
            "Nearby_" + arrival.train->name + "_" + arrival.ftime + "_" + std::to_string(i++),
            arrival.train->name,
            std::to_string(arrival.train->dirID),
            station.getfTime(),
            strtimeUntilArrival,
            arrival.ftime
        };
        sqlite::insert_row(db, table, data);
    }

    return tableName;
}

}