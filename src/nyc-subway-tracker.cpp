#include <iostream>
#include <fstream>

#include <string>
#include <vector>

#include <chrono>
    using namespace std::chrono_literals;
#include <thread>

// nyc-subway-tracker includes
#include <tracker.h>
#include <subway.h>
#include <line.h>
#include <station.h>


int main(int argc, char* argv[]) {
    unsigned snapshots_num = 3;
    std::string sleep_time_str = "1";
    std::chrono::minutes sleep_time(std::stoul(sleep_time_str));
    // TODO: error handling here and better argument parsing

    // get arguments from cmdline
    if (argc >= 3) { 
        snapshots_num = std::stoul(std::string(argv[1]));
        sleep_time_str = std::string(argv[2]);
        std::chrono::minutes sleep_time(std::stoul(sleep_time_str));
    } 

    sqlite3* db = nullptr; 
    db = tracker::subway_db_initialize(db); //initialize db and have db point to it

    Subway subway; //subway object that holds last system check status

    // std::ofstream file("subway.txt");
    // file << subway << std::endl;
    // file.close();

    std::cout << "Performing " << snapshots_num << " updates with " << 
        sleep_time_str << " between each update." << std::endl;
    for (auto i = 0; i < snapshots_num; i++) {
        std::cout << "Starting Update #" << i + 1 << "..." << std::endl;
        subway.update(); //update subway object -- get current system status
        
        std::cout << "Starting Snapshot #" << i + 1 << "..." << std::endl;
        tracker::snapshot(subway, db); //log current system status in db

        if (i != snapshots_num - 1) { //sleep if not last snapshot
            std::cout << "Sleeping for " << sleep_time_str << " minutes..." << std::endl;
            std::this_thread::sleep_for(sleep_time); //wait for 1 min before next update
        }
    }

    std::cout << "All snapshot data can be found in " + constant::DB_NAME << std::endl;
}