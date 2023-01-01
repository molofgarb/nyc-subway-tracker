#include <iostream>
#include <fstream>
#include <string>

#include "station.h"

// #include "nlohmann/single_include/nlohmann/json.hpp"
// using json = nlohmann::json;


int main() {
    // std::cout << "Begin" << std::endl;
    // std::ifstream routes_data("../resources/routes.json");

    // json routes = json::parse(routes_data);
    // std::cout << routes["id"];

    // test for Station

    std::vector<std::string> trainChars{"E", "F", "R", "M"};
    std::vector<Train*> trainTypes;
    for (size_t i = 0; i < 8; i++) {
        Train* train = new Train(trainChars[i % 4], int( (i < 4) ? 1 : 0) );
        trainTypes.push_back(train);
    }

    Station roosevelt("Jackson Hts-Roosevelt Av", "G14", &trainTypes);

    std::cout << roosevelt << std::endl;
}