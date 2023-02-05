#include <iostream>
#include <fstream>

#include <vector>
#include <string>
#include <ctime>

#include "nlohmann/single_include/nlohmann/json.hpp"

#include "getPage.h"

#include "Line.h"

const std::string LINE_URL = "https://collector-otp-prod.camsys-apps.com/schedule/MTASBWY/stopsForRoute?apikey=qeqy84JE7hUKfaI0Lxm2Ttcm6ZA0bYrP&&routeId=MTASBWY:";
const std::string LINE_API_KEY = "qeqy84JE7hUKfaI0Lxm2Ttcm6ZA0bYrP";

// constructor
Line::Line(const std::string& name, const std::vector<Train*>* trainTypes): 
           name(name), trainTypes(trainTypes) {
    const std::string url = LINE_URL + name;
    const std::vector<std::string> headers{
        "apikey: " + LINE_API_KEY,
        "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/avif,image/webp,*/*;q=0.8"
    };
    const std::string outFile = "lineTemp.json";

    get_page::getPage(url, headers, outFile);
    parseLineJSON(*this, outFile, trainTypes);
    std::remove("lineTemp.json");
}

// populates stations with a station pointer for each station in lineTemp.json
void parseLineJSON(Line& line, const std::string& filename, const std::vector<Train*>* trainTypes) {
    std::ifstream file(filename);
    nlohmann::json data = nlohmann::json::parse(file);
    
    // create a Station for each station in data
    for (const nlohmann::json& stationData : data) {
        line.stations.push_back(
            new Station(
                stationData["stopName"],
                stationData["stopId"].get<std::string>().substr(8, 3),
                trainTypes
            )
        );
    }
}

// copy constructor
Line::Line(const Line& other):
           name(other.name), trainTypes(other.trainTypes) {
    for (const Station* otherStation : other.stations) {
        stations.push_back(
            new Station(otherStation->getNameAndID().first,
                        otherStation->getNameAndID().second, 
                        other.trainTypes)
        );
    }
}

// assignment operator
Line& Line::operator=(const Line& other) {
    if (this != &other) {
        name = other.name;
        for (const Station* otherStation : other.stations) {
            stations.push_back(
                new Station(otherStation->getNameAndID().first,
                            otherStation->getNameAndID().second, 
                            other.trainTypes)
            );
        }
    }
    return *this;
}

// deconstructor
Line::~Line() {
    for (Station* station: stations) {
        delete station;
        station = nullptr;
    }
    stations.clear();
}

void Line::update() {
    for (Station* station : stations) {
        station->update();
    }
}

std::ostream& operator<<(std::ostream& os, const Line& rhs) {
    os << "Below are the statuses for each station on the " <<
           rhs.name << ":\n" << std::endl;
    for (Station* station : rhs.stations) {
        os << *station << std::endl;
    }
    return os;
}