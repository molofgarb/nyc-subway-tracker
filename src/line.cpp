#include <memory>

#include <iostream>
#include <fstream>

#include <vector>
#include <string>

#include <ctime>

// external includes
#include <nlohmann/json.hpp>

// nyc-subway-tracker includes
#include <common.h>
#include <station.h>

#include <line.h>

using st_ptr = std::shared_ptr<Station>;


// constructor
Line::Line(const std::string& name,
           const std::string& ID, 
           const std::map<Train*, int>* trainTypes,
           std::map<std::string, st_ptr>* allStations //boss of line
    ): name(name), ID(ID), trainTypes(trainTypes) {
    const std::string url = constant::LINE_URL + ID;
    const std::vector<std::string> headers{
        "apikey: " + constant::LINE_API_KEY,
    };
    std::string jsonData = "";
    get_page::get_page(url, headers, jsonData);
    parseLineJSON(jsonData, trainTypes, allStations);
}

// populates stations with a station pointer for each station in lineTemp.json
int Line::parseLineJSON(std::string& jsonData, 
                       const std::map<Train*, int>* trainTypes, 
                       std::map<std::string, st_ptr>* allStations
    ) {
    nlohmann::json data = nlohmann::json::parse(jsonData);
    
    // create a Station for each station in data
    for (const nlohmann::json stationData : data) {
        std::string stationID = stationData["stopId"].get<std::string>().substr(8, 3);
        if (allStations->find(stationID) != allStations->end()) { //station exists in allStations
            stations.push_back(allStations->at(stationID));
        } else { //station does not exist in allStations
            st_ptr station = st_ptr(
                new Station(
                    stationData["stopName"],
                    stationID,
                    trainTypes
                )
            );
            stations.push_back(station); //add to line's list of stations
            (*allStations)[stationID] = station; //add to all stations list kept by subway
        }
    }
    return 0;
}

// copy constructor
Line::Line(const Line& other):
           name(other.name), ID(other.ID), trainTypes(other.trainTypes) {
    for (auto station_ptr : other.stations) {
        st_ptr station = st_ptr(new Station(*station_ptr));
        stations.push_back(station);
    }
}

// assignment operator
Line& Line::operator=(const Line& other) {
    if (this != &other) {
        name = other.name;
        ID = other.ID;
        trainTypes = other.trainTypes;
        for (auto station_ptr : other.stations) {
            st_ptr station = st_ptr(new Station(*station_ptr));
            stations.push_back(station);
        }
    }
    return *this;
}

Line::~Line() { //Subway will manage station deletion (for now)
    stations.clear(); //st_ptr will manage own deletion
} 

//updates each station that belongs to the station
int Line::update() { //should typically not be used to avoid update overlap
    for (auto station : stations) {
        station->update();
    }
    return 0;
}

std::ostream& operator<<(std::ostream& os, const Line& rhs) {
    os << "Below are the statuses for each station on the " <<
           rhs.name << ":\n\n";
    for (auto station : rhs.stations) {
        os << *station << '\n';
    }
    return os << '\n';
}