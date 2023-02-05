#include <iostream>
#include <fstream>

#include <string>
#include <vector>

// #include "nlohmann/single_include/nlohmann/json.hpp"
// using json = nlohmann::json;

#include "subway.h"
#include "station.h"
#include "line.h"


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

    Subway subway;
    std::ofstream file("subway.txt");
    subway.update();
    file << subway << std::endl;

    // //debug for test
    // while (true) {
    //     std::cout << "Input: ";
    //     std::string input;
    //     std::cin >> input;
    //     subway.debug(input);
    //     std::cout << std::endl;
    // }

    // ===== CLEAN UP FOR STATION--LINE TESTS =================================

    // for (Train* train : trainTypes) {
    //     delete train;
    //     train = nullptr;
    // }
    // trainTypes.clear();
    // trainChars.clear();

}