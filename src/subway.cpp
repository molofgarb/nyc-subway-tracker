// external includes
#include <nlohmann/json.hpp>

// nyc-subway-tracker includes
#include <common.h>

#include <station.h>
#include <line.h>

#include <subway.h>

#define FILENAME "line.cpp"
#define DEBUG_UPDATETHREAD 0

using st_ptr = std::shared_ptr<Station>;

Subway::Subway() {
    lines.reserve(constant::SUBWAY_RESERVE_LINES);
    allStations.reserve(constant::SUBWAY_RESERVE_ALLSTATIONS);
    trainTypes.reserve(constant::SUBWAY_RESERVE_TRAINTYPES);

    const std::vector<std::string> headers{
        "apikey: " + constant::SUBWAY_API_KEY,
    };
    std::string data = "";

    if (get_page::get_page(constant::SUBWAY_URL, headers, data))
        common::panic(FILENAME, "Line::line", "curl"); 

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

            // add line to lines record and initialize that line
            lines.emplace_back(name, id, allStations, &trainTypes);
        }
    }
    return 0;
}

// =============================================================================

//update each station from stations
int Subway::update() {
    // initialize threads vector
    std::vector<std::thread> threads;
    threads.reserve(constant::THREADS);

    // spawn multiple threads to perform subway updates
    for (size_t offset = 0; offset < constant::THREADS; offset++)
        threads.emplace_back(&Subway::updateThread, this, offset);

    // wait for all threads to stop
    for (auto& thread : threads)
        thread.join();

    return 0;
}

int Subway::updateThread(size_t offset) {
    size_t i = 0;
    auto stationptr = allStations.begin();

    std::advance(stationptr, offset);
    i += offset;

    while (stationptr != allStations.end()) {

        // update a station
        (stationptr->second)->update();

        if (DEBUG_UPDATETHREAD)
            std::cout << "<debug> just finished an update in thread " 
                      << std::to_string(offset) << std::endl;

        // if we are safe to increment to next station, then do so
        // otherwise, signal to while loop that we are done
        if (i + constant::THREADS < allStations.size()) {
            std::advance(stationptr, constant::THREADS);
            i += constant::THREADS;
        } else {
            stationptr = allStations.end();
        }

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
        std::unordered_set<st_ptr> allStationsCheck{}; 

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