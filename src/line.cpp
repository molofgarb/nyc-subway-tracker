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
           std::map<std::string, st_ptr>& allStations, //boss of line
           const std::set<Train>* trainTypes
): name(name), ID(ID), trainTypes(trainTypes) {
    const std::string url = constant::LINE_URL + ID;
    const std::vector<std::string> headers{
        "apikey: " + constant::LINE_API_KEY
    };
    std::string jsonData = "";
    get_page::get_page(url, headers, jsonData);

    parseLineJSON(jsonData, allStations, trainTypes);
}

// populates stations with a station pointer for each station in lineTemp.json
int Line::parseLineJSON(std::string& jsonData, 
                        std::map<std::string, st_ptr>& allStations,
                        const std::set<Train>* trainTypes
) {
    nlohmann::json data = nlohmann::json::parse(jsonData);
    
    // create a Station for each station in data
    for (const nlohmann::json stationData : data) {
        std::string stationID = stationData["stopId"].get<std::string>().substr(8, 3);

        // station exists in allStations
        if (allStations.find(stationID) != allStations.end()) { 
            stations.push_back(allStations.at(stationID));

        // station does not exist in allStations
        } else { 
            st_ptr station = st_ptr(
                new Station(
                    stationData["stopName"],
                    stationID,
                    trainTypes
            ));
            stations.push_back(station);      // add to line's list of stations
            allStations[stationID] = station; // add to all stations list kept by subway
        }

    }
    return 0;
}

// =============================================================================

//updates each station that belongs to the station
int Line::update() { //should typically not be used to avoid update overlap
    for (auto station : stations) {
        station->update();
    }
    return 0;
}

// =============================================================================

std::ostream& operator<<(std::ostream& os, const Line& rhs) {
    os << "Below are the statuses for each station on the " <<
           rhs.name << ":\n\n";
    for (auto station : rhs.stations) {
        os << *station << '\n';
    }
    return os << '\n';
}