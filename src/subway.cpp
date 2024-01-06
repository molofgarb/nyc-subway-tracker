// external includes
#include <nlohmann/json.hpp>

// nyc-subway-tracker includes
#include <common.h>

#include <station.h>
#include <line.h>

#include <subway.h>

using st_ptr = std::shared_ptr<Station>;

Subway::Subway() {
    const std::vector<std::string> headers{
        "apikey: " + constant::SUBWAY_API_KEY,
    };
    std::string data = "";
    get_page::get_page(constant::SUBWAY_URL, headers, data);

    parseSubwayJSON(data);
}

// fills out lines and trainTypes with data from subwayTemp.json
int Subway::parseSubwayJSON(std::string& jsonData) {
    nlohmann::json data = nlohmann::json::parse(jsonData);

    // create trainTypes and lines
    // note: dont make the iterable const, nlohmann doesn't like it :(
    for (nlohmann::json& item : data) {
        // name is something like "R" for the R train
        // all shuttles are named S, but have different IDs
        // id is usually same as name (e.g. R) but can be different sometimes
        std::string id = item["id"].get<std::string>();
        std::string name = (item["shortName"].is_null()) ? "" : 
                                   item["shortName"].get<std::string>();
        
        // regular subway and not special express type of subway
        if ((id.substr(0, 8) == "MTASBWY:") && (id[id.size() - 1] != 'X')) { 
            
            // re-process ID and name if it is an actual subway line
            id = std::string(
                id.substr(8, int(id.size()) - 8)); //all characters after ":"
            name = (name == "S") ?
                constant::SHUTTLE_NAMES.at(id) : //if subway shuttle
                name; //if regular subway

            //add a train going in each direction to shared train types record
            trainTypes.emplace(name, 0); 
            trainTypes.emplace(name, 1);

            // add line to lines record
            lines.emplace_back(name, id, allStations, &trainTypes);
        }
    }
    return 0;
}

// =============================================================================

//update each station from stations
int Subway::update() {
    for (auto& stationPair : allStations) {
        (*stationPair.second).update();
    }
    return 0;
}

// =============================================================================

std::ostream& Subway::outputByLine(std::ostream& os, bool allowRepeat) const {
    // list out stations by line, allowing stations to be repeated if shared
    // by multiple lines
    if (allowRepeat) {
        for (const auto& line : lines) {
            os << line << '\n';
        }

    // do not repeat stations
    } else { 
        //tracks if station has been output
        std::set<st_ptr> allStationsCheck{}; 

        for (const auto& line : lines) {
            os << "Below are the statuses for each station on the " <<
                line.getName() << ":\n\n";

            std::vector<st_ptr> stations(line.getStations());

            for (const auto& stationptr : stations) {

                if (allStationsCheck.find(stationptr) == allStationsCheck.end()) {
                    os << *stationptr << '\n';
                    allStationsCheck.insert(stationptr);
                }

            }

        }

    }
    return os;
}

std::ostream& Subway::outputByStation(std::ostream& os) const {
    os << "These are the nearby trains for all stations." << '\n';

    time_t now = std::time(nullptr);
    os << "The current time is " << common::formatTime(&now) << '.' << '\n';

    os << "==================================================" << '\n';
    for (const auto& stationPair : allStations) {
        os << *(stationPair.second);
    }
    return os << '\n';
}

std::ostream& operator<<(std::ostream& os, const Subway& rhs) {
    return rhs.outputByStation(os); //default output
}