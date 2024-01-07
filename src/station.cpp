// external includes
#include <pugixml.hpp>

// nyc-subway-tracker includes
#include <common.h>

#include <station.h>

#define FILENAME "station.cpp"
                 
// ===== STATION ==============================================================

//Station constructor
Station::Station(const std::string& name, 
                 const std::string& stopID, 
                 const std::set<Train>* trainTypes): 
                 name(name), stopID(stopID), update_time(0), 
                 update_ftime(""), trainTypes(trainTypes) {}

// =============================================================================

int Station::update() {
    nearby.clear();

    //get xml with arrival data
    const std::vector<std::string> headers{ //headers in get request
        "apikey: " + constant::STATION_API_KEY,
        "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/avif,image/webp,*/*;q=0.8"
    };
    std::string data = "";
    get_page::get_page(constant::STATION_URL + stopID, headers, data); //write xml page to xmlData
    
    // parse xml data
    pugi::xml_document doc;
    pugi::xml_parse_result result = doc.load_buffer_inplace(data.data(), data.size());
    if (!result)
        common::panic(FILENAME, "update");

    //update vector nearby with train info
    populateNearby(doc); 

    update_time = time(nullptr);
    update_ftime = common::formatTime(&update_time);
    return 0;
} 

//populates nearby vector with info on nearby trains using an XML file in local dir
int Station::populateNearby(pugi::xml_document& doc) {
    pugi::xml_node root = doc.child("LinkedValues");

    // for each type of train (ex. Manhattan-bound F)
    for (auto& incomingTrainType: 
         root.child("item").child("groups").children("groups") 
    ) {
        // get info about the type of train
        std::string name = incomingTrainType.child("route").child("id").child("id").text().as_string();
        int dirID = incomingTrainType.child("times").child("times").child("directionId").text().as_int();
        std::string headsign = incomingTrainType.child("headsign").text().as_string();

        // swap name with name on MTA map if necessary
        if (constant::SHUTTLE_NAMES.find(name) != constant::SHUTTLE_NAMES.end()) //if name exists
            name = constant::SHUTTLE_NAMES.at(name); //swap name (assuming equal to ID) with actual name

        // remove special express indicator
        if (name[name.size() - 1] == 'X') 
            name = name.substr(0, name.size() - 1);

        // match train type to known train pointer
        const Train* trainptr = &(*( trainTypes->find(Train(name, dirID)) )); //pointer to train type being checked
        if ( trainptr == &*(trainTypes->end()) ) 
            common::panic(FILENAME, "populateNearby", name + " " + std::to_string(dirID));

        //check incomings of this type and add to nearby vector
        for (auto& incomingTrain: incomingTrainType.child("times").children("times")) { 
            time_t time = //in unix time, realtime is the number used on webpage
                    incomingTrain.child("serviceDay").text().as_int() +
                    incomingTrain.child("realtimeArrival").text().as_int(); 

            nearby.emplace_back(
                trainptr, headsign, time, common::formatTime(&time)
            );
        }
    }
    return 0;
}

// =============================================================================

// break glass in case of emergency
std::ostream& operator<<(std::ostream& os, const Station& rhs) {
    std::time_t timeNow = std::time(nullptr);
    char timestr[128] = "";

    os << "The current station is " << rhs.name << ", " << rhs.stopID << '.' << '\n';

    if (std::strftime(timestr, sizeof(timestr), "%I:%M:%S %p", std::localtime(&timeNow)))
        os << "The current time is " << timestr << '.' << '\n';
    if (std::strftime(timestr, sizeof(timestr), "%I:%M:%S %p", std::localtime(&rhs.update_time)))
        os << "The data was updated for this station at " << timestr << '.' << '\n';

    for (const auto& arrival : rhs.nearby) {
        std::time_t untilArrivalTime = arrival.time - rhs.update_time; //diff arrival to now time
        std::time_t arrivalTime(int(arrival.time) % (86400)); //updateTime in sec from start of day

        os << arrival.train->name << ", " << 
            ((arrival.train->dirID > 0) ?  "southbound (1)" : "northbound (0)");
        if (std::strftime(timestr, sizeof(timestr), "%M:%S", std::localtime(&untilArrivalTime)))
            os << ",\t" << "Time until arrival from now: " << timestr;
        if (std::strftime(timestr, sizeof(timestr), "%I:%M:%S %p", std::localtime(&arrivalTime)))
            os << ",\t" << "Arrival Time: " << timestr;
        os << '\n';

    }
    return os << '\n';
}
