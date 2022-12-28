#include <iostream>
#include <fstream>
#include <string>

#include "nlohmann\json.hpp"
using json = nlohmann::json;


int main() {
    std::cout << "Hello World" << std::endl;
    std::ifstream routes_data("../resources/routes.json");

    json routes = json::parse(routes_data);
    std::cout << routes["id"];
}