// external includes
#include <nlohmann/json.hpp>

// nyc-subway-tracker includes
#include <common.h>

#include <station.h>

#include <line.h>

#define FILENAME "line.cpp"

using st_ptr = std::shared_ptr<Station>;

// constructor
Line::Line(
    const std::string& name,
    const std::string& ID, 
    std::unordered_map<std::string, st_ptr>& all_stations, //boss of line
    const std::unordered_set<Train, NSThash>* train_types
    ): name(name), ID(ID), train_types(train_types) {

    stations.reserve(constant::LINE_RESERVE_STATION);

    const std::vector<std::string> headers{
        "apikey: " + constant::LINE_API_KEY
    };
    std::string data = "";

    if (get_page::get_page(constant::LINE_URL + ID, headers, data))
        common::panic(FILENAME, "Line::line", "curl"); 

    parseLineJSON(data, all_stations);
}

// populates stations with a station pointer for each station in lineTemp.json
int Line::parseLineJSON(
    std::string& jsonData, 
    std::unordered_map<std::string, st_ptr>& all_stations) {
        
    nlohmann::json data = nlohmann::json::parse(jsonData);
    
    // create a Station for each station in data
    for (const nlohmann::json& stationData : data) {
        std::string stationID = stationData["stopId"].get<std::string>().substr(8, 3);

        // station exists in all_stations
        if (all_stations.find(stationID) != all_stations.end()) { 
            stations.push_back(all_stations.at(stationID));

        // station does not exist in all_stations
        } else { 
            if (DEBUG)
                std::cout << "<debug> <line.cpp> <parseLineJSON> making station: " 
                        << stationID << std::endl;
            st_ptr station = st_ptr(
                new Station(
                    stationData["stopName"],
                    stationID,
                    train_types
            ));
            stations.push_back(station);      // add to line's list of stations
            all_stations[stationID] = station; // add to all stations list kept by subway
        }

    }
    return 0;
}

// =============================================================================

//updates each station that belongs to the station
int Line::update() { //should typically not be used to avoid update overlap
    for (auto& station : stations) {
        station->update();
    }
    return 0;
}

// =============================================================================

std::ostream& operator<<(std::ostream& os, const Line& rhs) {
    os << "Below are the statuses for each station on the " <<
           rhs.name << ":\n\n";
    for (const auto& station : rhs.stations) {
        os << *station << '\n';
    }
    return os << '\n';
}