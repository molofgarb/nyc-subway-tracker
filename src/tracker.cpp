#include <pch.h>

// external includes
#include <sqlite3.h>

// nyc-subway-tracker includes
#include <tracker_sqlite.h>
#include <subway.h>
#include <line.h>
#include <station.h>

#include <tracker.h>

#define FILENAME __builtin_FILE()

namespace tracker {

// ===== FUNCTIONS =============================================================

//takes a snapshot -- adds current system status table into db snapshot
//Creates table Subway_Snapshots using constant::SNAPSHOT_TABLE
int snapshot(const Subway& subway, const std::string& db_name) {
    TSqlite db(db_name, std::time(nullptr));

    // reserves enough space in statement buffer for big statement coming up
    db.reserveSqliteStatementBuf(constant::SQLITE_RESERVE_STATEMENT_BUF);

    // prepare snapshot data, perform snapshot stepping through subway, then execute 
    db.createNewTable(SNAPSHOT_TABLE);
    std::vector<std::string> data{
        subwaySnapshot(db, subway),

        "Subway",

        common::formatTime(db.getTimePtr()),
        std::to_string(db.getTime())
    };
    db.insertRow(SNAPSHOT_TABLE, data);

    // execute all statements stored in the buffer
    // there should be a lot of statements
    db.execStatements();

    return 0;
} 

// ----------------------------------------------------------------------------

// logs current subway system status as a table with station snapshots 
// Creates tables like Subway_Stations_1673908573 
std::string subwaySnapshot(TSqlite& db, const Subway& subway) {

    //make table for subway table that holds stations
    const std::string table_name = "Subway_" + 
        common::formatTime(db.getTimePtr(), common::SQLITE);

    //table containing every station in snapshot
    Table table(
        table_name,
        SUBWAY_COLUMNS
    ); 
    db.createNewTable(table);

    // add an entry for each station
    const std::unordered_map<std::string, st_ptr> all_stations = 
        subway.getStations();

    size_t num_threads = constant::THREADS;
    
    // initialize threads vector
    std::vector<std::thread> threads;
    threads.reserve(num_threads);

    // spawn multiple threads
    for (size_t offset = 0; offset < num_threads; offset++)
        threads.emplace_back(&tracker::subwaySnapshotThread, 
            &db, all_stations, table, offset, num_threads);

    // wait for all threads to stop
    for (auto& thread : threads)
        thread.join();

    // add an entry for each line
    // there aren't many lines so this probably doesn't need to be multithreaded
    for (const auto& line : subway.getLines()) {
        std::vector<std::string> data{
            lineSnapshot(db, line),
            "Line",
            line.getName()
        };
        db.insertRow(table, data);
    }

    return table_name;
}

// performs subway snapshotting for one thread
int subwaySnapshotThread(
    TSqlite* db, 
    const std::unordered_map<std::string, st_ptr>& all_stations, 
    const Table& table, 
    size_t offset,
    size_t num_threads) {

    size_t i = 0;
    auto stationptr = all_stations.begin();

    i += offset;
    std::advance(stationptr, offset);
    while (stationptr != all_stations.end()) {
        std::vector<std::string> data{
            stationSnapshot(*db, *stationptr->second),
            "Station",
            (*stationptr).second->getName()
        };
        db->insertRow(table, data);

        // if we are safe to increment to next station, then do so
        // otherwise, signal to while loop that we are done
        if (i + num_threads < all_stations.size()) {
            i += num_threads;
            std::advance(stationptr, num_threads);
        } else {
            stationptr = all_stations.end();
        }
    }
    return 0;
}

std::string lineSnapshot(
    TSqlite& db, 
    const Line& line, 
    bool SNAPSHOT_STATIONS) {

    // make table for line
    const std::string table_name = "Line_" + line.getName()  + "_" + 
        common::formatTime(db.getTimePtr(), common::SQLITE);
    Table table = Table(
        table_name,
        LINES_COLUMNS
    );
    db.createNewTable(table);

    // add entries for each station in line
    // station update NOT performed -- subway_snapshot will do it
    for (const auto& station : line.getStations()) {
        std::string stationName;

        // choose whether to update the station or not
        if (SNAPSHOT_STATIONS) 
            stationName = tracker::stationSnapshot(db, *station);
        else 
            stationName = "Station_" + station->getID() + "_" + 
                common::formatTime(db.getTimePtr());

        std::vector<std::string> data{
            stationName,
            station->getID(),
            station->getName(),

            station->getfTime(),
            std::to_string(station->getTime())
        };
        db.insertRow(table, data); // add new station table into subway snapshot
    }

    return table_name;
}

// logs current station status as a table with nearby arrivals as contents
// Creates tables like Station_G14_1673908573 using constant::STATION_COLUMNS
std::string stationSnapshot(TSqlite& db, const Station& station) {

    //make table for station
    const std::string table_name = "Station_" + station.getID() + "_" + 
        common::formatTime(db.getTimePtr(), common::SQLITE);
    Table table = Table(
        table_name,
        STATION_COLUMNS
    );
    db.createNewTable(table);

    //add entries for each nearby arrival
    for (const auto& arrival : station.getNearby()) {
        time_t timeUntilArrival = arrival.time - station.getTime();

        // convert time until arrival
        // if the time until arrival is negative, just report it as N/A
        std::string strtimeUntilArrival = "N/A";
        if (timeUntilArrival >= 0)
            strtimeUntilArrival = common::formatTime(&timeUntilArrival, common::MINSEC);

        std::vector<std::string> data{
            "Nearby_" + arrival.train->name + "_" + 
                std::to_string(arrival.train->dirID) + "_" + arrival.ftime,

            arrival.train->name,
            std::to_string(arrival.train->dirID),
            arrival.headsign,

            station.getfTime(),
            strtimeUntilArrival,
            arrival.ftime,

            std::to_string(station.getTime()),
            std::to_string(timeUntilArrival),
            std::to_string(arrival.time)
        };
        db.insertRow(table, data);
    }

    return table_name;
}

}