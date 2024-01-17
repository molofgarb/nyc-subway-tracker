#include <pch.h>

// external includes
#include <nlohmann/json.hpp>

// nyc-subway-tracker includes
#include <station.h>
#include <line.h>

#include <subway.h>

#define FILENAME __builtin_FILE()
#define DEBUG_UPDATETHREAD 0

using st_ptr = std::shared_ptr<Station>;

const std::string Subway::SUBWAY_URL = 
    "https://otp-mta-prod.camsys-apps.com/otp/routers/default/index/routes";
const std::string Subway::SUBWAY_API_KEY = 
    "Z276E3rCeTzOQEoBPPN4JCEc6GfvdnYE";

Subway::Subway() {
    lines.reserve(constant::SUBWAY_RESERVE_LINES);
    all_stations.reserve(constant::SUBWAY_RESERVE_ALL_STATIONS);
    train_types.reserve(constant::SUBWAY_RESERVE_TRAIN_TYPES);

    const std::vector<std::string> headers{
        "apikey: " + SUBWAY_API_KEY,
    };
    std::string data = "";

    if (get_page::get_page(SUBWAY_URL, headers, data))
        common::panic("curl"); 

    parseSubwayJSON(data);
}

// fills out lines and train_types with data from subwayTemp.json
int Subway::parseSubwayJSON(std::string& jsonData) {
    
    // use nlohmann's library to parse JSON data
    nlohmann::json data;
    try { data = nlohmann::json::parse(jsonData); }
    catch (std::exception& e) { common::panic("parse error"); }

    // create train_types and lines
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
            train_types.emplace(name, 0); 
            train_types.emplace(name, 1);

            // add line to lines record and initialize that line
            lines.emplace_back(name, id, all_stations, &train_types);
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
    auto stationptr = all_stations.begin();

    i += offset;
    std::advance(stationptr, offset);

    while (stationptr != all_stations.end()) {

        // update a station
        (stationptr->second)->update();

        if (DEBUG_UPDATETHREAD)
            std::cout << "<debug> just finished an update in thread " 
                      << std::to_string(offset) << std::endl;

        // if we are safe to increment to next station, then do so
        // otherwise, signal to while loop that we are done
        if (i + constant::THREADS < all_stations.size()) {
            i += constant::THREADS;
            std::advance(stationptr, constant::THREADS);
        } else {
            stationptr = all_stations.end();
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
        std::unordered_set<st_ptr> all_stationsCheck{}; 

        for (const auto& line : lines) {
            os << "Below are the statuses for each station on the " <<
                line.getName() << ":\n\n";

            std::vector<st_ptr> stations(line.getStations());

            for (const auto& stationptr : stations) {

                if (all_stationsCheck.find(stationptr) == all_stationsCheck.end()) {
                    os << *stationptr << '\n';
                    all_stationsCheck.insert(stationptr);
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
    for (const auto& stationPair : all_stations) {
        os << *(stationPair.second);
    }
    return os << '\n';
}

std::ostream& operator<<(std::ostream& os, const Subway& rhs) {
    return rhs.outputByStation(os); //default output
}