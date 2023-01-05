#include <iostream>
#include <fstream>

#include <vector>
#include <string>
#include <ctime>

#include "nlohmann/single_include/nlohmann/json.hpp"

#include "getPage.h"
#include "Station.h"

#include "Line.h"


// constructor
Line::Line(const std::string& name,
           const std::string& ID, 
           const std::map<Train*, int>* trainTypes,
           std::map<std::string, Station*>* allStations //boss of line
    ): name(name), ID(ID), trainTypes(trainTypes) {
    const std::string url = constant::LINE_URL + ID;
    const std::vector<std::string> headers{
        "apikey: " + constant::LINE_API_KEY,
    };
    const std::string outFile = "lineTemp.json";
    get_page::getPage(url, headers, outFile);
    parseLineJSON(outFile, trainTypes, allStations);
    std::remove("lineTemp.json");
}

// populates stations with a station pointer for each station in lineTemp.json
void Line::parseLineJSON(const std::string& filename, 
                       const std::map<Train*, int>* trainTypes, 
                       std::map<std::string, Station*>* allStations
    ) {
    std::ifstream file(filename);
    nlohmann::json data = nlohmann::json::parse(file);
    
    // create a Station for each station in data
    for (const nlohmann::json stationData : data) {
        std::string stationID = stationData["stopId"].get<std::string>().substr(8, 3);
        if (allStations->find(stationID) != allStations->end()) { //station exists in allStations
            stations.push_back(allStations->at(stationID));
        } else { //station does not exist in allStations
            Station* station = new Station(
                stationData["stopName"],
                stationID,
                trainTypes
            );
            stations.push_back(station); //add to line's list of stations
            (*allStations)[stationID] = station; //add to all stations list kept by subway
        }
    }
}

// copy constructor
Line::Line(const Line& other):
    name(other.name), ID(other.ID), stations(other.stations), trainTypes(other.trainTypes) {}

// assignment operator
Line& Line::operator=(const Line& other) {
    if (this != &other) {
        name = other.name;
        ID = other.ID;
        stations = other.stations;
        trainTypes = other.trainTypes;
    }
    return *this;
}

Line::~Line() { //Subway will manage station deletion (for now)
    stations.clear();
} 

// Subway will handle memory management, commented out individual-style functions
// // copy constructor
// Line::Line(const Line& other):
//            name(other.name), ID(other.ID), trainTypes(other.trainTypes) {
//     for (const Station* otherStation : other.stations) {
//         stations.push_back(
//             new Station(otherStation->getNameAndID().first,
//                         otherStation->getNameAndID().second, 
//                         other.trainTypes)
//         );
//     }
// }

// // assignment operator
// Line& Line::operator=(const Line& other) {
//     if (this != &other) {
//         name = other.name;
//         ID = other.ID;
//         for (const Station* otherStation : other.stations) {
//             stations.push_back(
//                 new Station(otherStation->getNameAndID().first,
//                             otherStation->getNameAndID().second, 
//                             other.trainTypes)
//             );
//         }
//     }
//     return *this;
// }

// // deconstructor (please implement with shared pointer for Station*)
// Line::~Line() { //subway will handle the Stations
//     for (Station* station: stations) {
//         if (station != nullptr) {
//             delete station;
//             station = nullptr;
//         }
//     }
//     stations.clear();
// }

//updates each station that belongs to the station
void Line::update() { //should typically not be used to avoid update overlap
    for (Station* station : stations) {
        station->update();
    }
}

const std::vector<Station*>* Line::getStations() const {
    return &stations;
}

std::string Line::getName() const {
    return name;
}

std::string Line::getID() const {
    return ID;
}

std::ostream& operator<<(std::ostream& os, const Line& rhs) {
    if (rhs.stations.size() == 0) return os;
    os << "Below are the statuses for each station on the " <<
           rhs.name << ":\n" << std::endl;
    for (Station* station : rhs.stations) {
        os << *station << std::endl;
    }
    return os;
}