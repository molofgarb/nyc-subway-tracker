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


int main() {
    // ===== FOR STATION--LINE TESTS ==========================================

    // std::vector<std::string> trainChars;
    // for (size_t i = 0; i < 10; i++) trainChars.push_back(std::to_string(int(i))); //single-digit integers
    // for (char i = 'A'; i < '['; i++) trainChars.push_back(std::string(1, i)); //capital letters A-Z
    // // for (std::string i : trainChars) std::cout << i << std::endl;

    // std::vector<Train*> trainTypes;
    // for (size_t i = 0; i < (trainChars.size() * 2); i++) {
    //     trainTypes.push_back(
    //         new Train(
    //             trainChars[i % trainChars.size()], 
    //             int(i / trainChars.size())
    //         )
    //     );
    // }

    // ===== STATION TEST =====================================================

    // Station roosevelt("Jackson Hts-Roosevelt Av", "G14", &trainTypes);
    // roosevelt.update();
    // std::cout << roosevelt << std::endl;

    // ===== JSON TEST ========================================================

    // std::cout << "Begin" << std::endl;
    // std::ifstream routes_data("stopsForRoute.json");

    // nlohmann::json routes = nlohmann::json::parse(routes_data);
    // for (auto& x : routes) {
    //     std::cout << x << std::endl;
    // }

    // ===== LINE TEST ========================================================

    // Line line("F", &trainTypes);
    // line.update();
    // std::cout << line << std::endl;

    // ===== SUBWAY TEST ======================================================

    // Subway subway;

    // std::ofstream file("subway.txt");
    // // std::ofstream file2("subway_repeat.txt");
    // // std::ofstream file3("subway_noRepeat.txt");

    // subway.update();

    // file << subway << std::endl;
    // // subway.outputByLine(file2, true);
    // // subway.outputByLine(file3, false);

    // // //debug for test
    // // while (true) {
    // //     std::cout << "Input: ";
    // //     std::string input;
    // //     std::cin >> input;
    // //     subway.debug(input);
    // //     std::cout << std::endl;
    // // }

    // ===== TRACKER TEST =====================================================

    sqlite3* db = nullptr; 
    db = tracker::subway_db_initialize(db); //initialize db and have db point to it

    Subway subway; //subway object that holds last system check status

    subway.update(); //update subway object -- get current system status
    std::cout << "Update #0 done, starting snapshot" << std::endl;

    tracker::snapshot(subway, db); //log current system status in db
    std::cout << "Snapshot #0 done!" << std::endl;

    // std::ofstream file("subway.txt");
    // file << subway << std::endl;
    // file.close();

    // for (auto i = 0; i < 10; i++) {
    //     std::cout << "Starting Update #" << i << "..." << std::endl;
    //     subway.update(); //update subway object -- get current system status
        
    //     std::cout << "Starting Snapshot #" << i << "..." << std::endl;
    //     tracker::snapshot(subway, db); //log current system status in db

    //     std::cout << "Sleeping..." << std::endl;
    //     std::this_thread::sleep_for(1min); //wait for 1 min before next update
    // }

    std::cout << "All snapshot data can be found in " + constant::DB_NAME << std::endl;

    // ===== CLEAN UP FOR STATION--LINE TESTS =================================

    // for (Train* train : trainTypes) {
    //     delete train;
    //     train = nullptr;
    // }
    // trainTypes.clear();
    // trainChars.clear();

}