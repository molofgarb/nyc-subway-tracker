#include <iostream>
#include <fstream>

#include <string>
#include <utility>

#include <ctime>

// external includes
#include <sqlite3.h>

// nyc-subway-tracker includes
#include <tracker_sqlite.h>
#include <subway.h>
#include <line.h>
#include <station.h>

#include <tracker.h>


namespace tracker {

// ----- Helper Functions -----------------------------------------------------
//gets current subway status in terms of station snapshot s
const std::string subway_station_snapshot(const Subway& subway, sqlite3* db, time_t time);

//gets current subway status in terms of lines snapshots
const std::string subway_line_snapshot(const Subway& subway, sqlite3* db, time_t time);

//gets current line status as a bunch of station snapshots
const std::string line_snapshot(const Line& line, sqlite3* db, time_t time);

//gets current station snapshot
const std::string station_snapshot(const Station& station, sqlite3* db, time_t time);
// ----------------------------------------------------------------------------

//initializes the database files used to keep track of snapshots
sqlite3* subway_db_initialize(sqlite3* db) {
    db = sqlite::open_db(db, constant::DB_NAME);

    db = sqlite::create_new_table(db, constant::SNAPSHOT_TABLE);

    return db;
}

//takes a snapshot -- adds current system status table into db snapshot
//Creates table Subway_Snapshots using constant::SNAPSHOT_TABLE
sqlite3* snapshot(const Subway& subway, sqlite3* db) {
    time_t time = std::time(nullptr);

    std::vector<std::string> stationData{
        tracker::subway_station_snapshot(subway, db, time),
        std::to_string((int)time)
    };
    sqlite::insert_row(db, constant::SNAPSHOT_TABLE, stationData);

    std::vector<std::string> lineData{
        tracker::subway_line_snapshot(subway, db, time),
        std::to_string((int)time)
    };
    sqlite::insert_row(db, constant::SNAPSHOT_TABLE, lineData);

    return db;
} 


// ----------------------------------------------------------------------------


//logs current subway system status as a table with station snapshots as contents
//Creates tables like Subway_Stations_1673908573 using constant::SUBWAY_STATIONS_COLUMNS
const std::string subway_station_snapshot(const Subway& subway, sqlite3* db, time_t time) {
    if (db == nullptr) { //db doesnt exist
        std::cerr << "<error> no db at subway_snapshot" << std::endl;
    }

    //make table for subway table that holds stations
    std::string timeStr = std::to_string((int)time); //time right now
    const std::string tableName = "Subway_Stations_" + timeStr;
    Table table(
        tableName,
        constant::SUBWAY_STATIONS_COLUMNS
    ); //table containing every station in snapshot
    db = sqlite::create_new_table(db, table);

    //add entries for each station
    for (const auto stationPair : *subway.getStations()) { 
        std::string stationTime = std::to_string(stationPair.second->getTime());

        //updates station, and then uses time of station update in primary key
        std::vector<std::string> data{
            tracker::station_snapshot(*stationPair.second, db, time), 
            stationPair.second->getID(),
            stationPair.first, //name
            stationTime
        };

        sqlite::insert_row(db, table, data); //add new station table into subway snapshot
    }

    return tableName;
}

//logs current subway system status as a table with line snapshots as contents
//Creates tables like Subway_Lines_1673908573 using constant::SUBWAY_LINES_COLUMNS
const std::string subway_line_snapshot(const Subway& subway, sqlite3* db, time_t time) {
    if (db == nullptr) { //db doesnt exist
        std::cerr << "<error> no db at subway_line_snapshot" << std::endl;
    }

    //make table for subway table that holds lines
    std::string timeStr = std::to_string((int)time); //time right now
    const std::string tableName = "Subway_Lines_" + timeStr;
    Table table(
        tableName,
        constant::SUBWAY_LINES_COLUMNS
    ); //table containing every line in snapshot
    db = sqlite::create_new_table(db, table);

    //add entries for each line
    for (const auto linePtr : subway.getLines()) {
        std::vector<std::string> data{
            tracker::line_snapshot(*linePtr, db, time),
            linePtr->getName(),
            timeStr
        };
        sqlite::insert_row(db, table, data);
    }

    return tableName;
}

const std::string line_snapshot(const Line& line, sqlite3* db, time_t time) {
    if (db == nullptr) { //db doesnt exist
        std::cerr << "<error> no db at line_snapshot" << std::endl;
    }

    //make table for line
    std::string timeStr = std::to_string((int)time); //time right now
    const std::string tableName = "Line_" + line.getName() + "_" + timeStr;
    Table table = Table(
        tableName,
        constant::LINES_COLUMNS
    );
    db = sqlite::create_new_table(db, table);

    //add entries for each station in line
    for (const auto station : line.getStations()) {
        //station update not performed -- subway_snapshot will do it
        std::string stationTime = std::to_string(station->getTime());
        std::vector<std::string> data{
            "Station_" + station->getID() + "_" + std::to_string(station->getTime()), 
            station->getID(),
            station->getName(),
            stationTime
        };

        sqlite::insert_row(db, table, data); //add new station table into subway snapshot
    }

    return tableName;
}

//logs current station status as a table with nearby arrivals as contents
//Creates tables like Station_G14_1673908573 using constant::STATION_COLUMNS
const std::string station_snapshot(const Station& station, sqlite3* db, time_t time) {
    if (db == nullptr) {
        std::cerr << "<error> no db at station_snapshot" << std::endl;
    }

    //make table for station
    std::string timeStr = std::to_string((int)time); //time right now
    const std::string tableName = "Station_" + station.getID() + "_" + timeStr;
    Table table = Table(
        tableName,
        constant::STATION_COLUMNS
    );
    db = sqlite::create_new_table(db, table);

    //time that station was checked conversion and formatting
    std::time_t updateTimeTemp = station.getTime();
    char updateTimeStr[128] = "";
    std::strftime(updateTimeStr, sizeof(updateTimeStr), "%I:%M:%S %p", std::localtime(&updateTimeTemp));

    //add entries for each nearby arrival
    for (const auto arrival : station.getNearby()) {
        std::string name = (*arrival.train).name;
        std::string arrivalTime = std::to_string(arrival.time);

        //time until arrival conversion and formatting
        std::time_t untilArrivalTimeTemp = arrival.time - station.getTime(); //diff arrival to now time
        char untilArrivalTimeStr[128] = "";
        std::strftime(untilArrivalTimeStr, sizeof(untilArrivalTimeStr), "%M:%S", std::localtime(&untilArrivalTimeTemp));

        //exact arrival time conversion and formatting
        std::time_t arrivalTimeTemp(int(arrival.time) % (86400)); //updateTime in sec from start of day
        char arrivalTimeStr[128] = "";
        std::strftime(arrivalTimeStr, sizeof(arrivalTimeStr), "%I:%M:%S %p", std::localtime(&arrivalTimeTemp));

        std::vector<std::string> data{
            name + "_" + arrivalTime, 
            name,
            std::string(updateTimeStr),
            std::string(untilArrivalTimeStr),
            std::string(arrivalTimeStr),
            std::to_string(station.getTime()),
            arrivalTime,
            std::to_string((*arrival.train).dirID), 
        };

        sqlite::insert_row(db, table, data);
    }

    return tableName;
}

}