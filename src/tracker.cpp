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
int snapshotDBInitialize(SqliteEnv& env, const std::string& db_name) {
    env.db = sqlite::openDB(db_name);
    sqlite::createNewTable(env, constant::SNAPSHOT_TABLE);

    return 0;
}

//takes a snapshot -- adds current system status table into db snapshot
//Creates table Subway_Snapshots using constant::SNAPSHOT_TABLE
int snapshot(const Subway& subway, const std::string& db_name) {
    // reserves enough space in statement buffer for big statement coming up
    sqlite::reserveSqliteStatementBuf(constant::SQLITE_RESERVE_STATEMENT_BUF);

    // initialize the database environment
    SqliteEnv env;

    // initialize db, time, and mutex of environment
    std::mutex mutex;
    snapshotDBInitialize(env, db_name);    
    env.time = std::time(nullptr);
    env.mutex = &mutex;

    // prepare snapshot data, perform snapshot stepping through subway, then execute 
    std::vector<std::string> data{
        tracker::subwaySnapshot(subway, env),

        "Subway",

        common::formatTime(&env.time),
        std::to_string(env.time)
    };
    sqlite::insertRow(env, constant::SNAPSHOT_TABLE, data);

    // execute all statements stored in the buffer
    // there should be a lot of statements
    sqlite::execzSql(env, "snapshot");

    return 0;
} 

// ----------------------------------------------------------------------------

//logs current subway system status as a table with station snapshots as contents
//Creates tables like Subway_Stations_1673908573 using constant::SUBWAY_STATIONS_COLUMNS
std::string subwaySnapshot(const Subway& subway, const SqliteEnv& env) {
    if (env.db == nullptr || env.time == 0)
        common::panic(FILENAME, "subway_snapshot", "no db or bad time");

    //make table for subway table that holds stations
    const std::string table_name = "Subway_" + common::formatTime(&env.time, common::SQLITE);

    //table containing every station in snapshot
    Table table(
        table_name,
        constant::SUBWAY_COLUMNS
    ); 
    sqlite::createNewTable(env, table);

    // add an entry for each station
    const std::unordered_map<std::string, st_ptr> all_stations = subway.getStations();

    size_t num_threads = constant::THREADS;

    // if no mutex is provided, do not multithread lest there be race condition
    if (env.mutex == nullptr)
        num_threads = 1;
    
    // initialize threads vector
    std::vector<std::thread> threads;
    threads.reserve(num_threads);

    // spawn multiple threads
    for (size_t offset = 0; offset < num_threads; offset++)
        threads.emplace_back(&tracker::subwaySnapshotThread, 
            all_stations, env, table, offset, num_threads);

    // wait for all threads to stop
    for (auto& thread : threads)
        thread.join();

    // add an entry for each line
    // there aren't many lines so this probably doesn't need to be multithreaded
    for (const auto& line : subway.getLines()) {
        std::vector<std::string> data{
            tracker::lineSnapshot(line, env),
            "Line",
            line.getName()
        };
        sqlite::insertRow(env, table, data);
    }

    return table_name;
}

// performs subway snapshotting for one thread
int subwaySnapshotThread(
    const std::unordered_map<std::string, st_ptr>& all_stations, 
    const SqliteEnv& env, 
    const Table& table, 
    size_t offset,
    size_t num_threads) {

    size_t i = 0;
    auto stationptr = all_stations.begin();

    i += offset;
    std::advance(stationptr, offset);
    while (stationptr != all_stations.end()) {
        std::vector<std::string> data{
            tracker::stationSnapshot(*(*stationptr).second, env),
            "Station",
            (*stationptr).second->getName()
        };
        sqlite::insertRow(env, table, data);

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

std::string lineSnapshot(const Line& line, const SqliteEnv& env) {
    if (env.db == nullptr || env.time == 0)
        common::panic(FILENAME, "lines_snapshot", "no db or bad time");

    // make table for line
    const std::string table_name = "Line_" + line.getName()  + "_" + common::formatTime(&env.time, common::SQLITE);
    Table table = Table(
        table_name,
        constant::LINES_COLUMNS
    );
    sqlite::createNewTable(env, table);

    // add entries for each station in line
    // station update NOT performed -- subway_snapshot will do it
    for (const auto& station : line.getStations()) {
        std::string stationName;

        // choose whether to update the station or not
        if (SNAPSHOT_STATIONS) 
            stationName = tracker::stationSnapshot(*station, env);
        else 
            stationName = "Station_" + station->getID() + "_" + common::formatTime(&env.time);

        std::vector<std::string> data{
            stationName,
            station->getID(),
            station->getName(),

            station->getfTime(),
            std::to_string(station->getTime())
        };
        sqlite::insertRow(env, table, data); // add new station table into subway snapshot
    }

    return table_name;
}

// logs current station status as a table with nearby arrivals as contents
// Creates tables like Station_G14_1673908573 using constant::STATION_COLUMNS
std::string stationSnapshot(const Station& station, const SqliteEnv& env) {
    if (env.db == nullptr || env.time == 0)
        common::panic(FILENAME, "subway_station_snapshot", "no db or bad time");

    //make table for station
    const std::string table_name = "Station_" + station.getID() + "_" + common::formatTime(&env.time, common::SQLITE);
    Table table = Table(
        table_name,
        constant::STATION_COLUMNS
    );
    sqlite::createNewTable(env, table);

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
        sqlite::insertRow(env, table, data);
    }

    return table_name;
}

}