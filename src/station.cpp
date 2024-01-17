#include <pch.h>

// external includes
#include <pugixml.hpp>

// nyc-subway-tracker includes
#include <station.h>
                 
// ===== STATION ==============================================================

const std::string Station::STATION_URL = 
    "https://otp-mta-prod.camsys-apps.com/otp/routers/default/nearby?stops=MTASBWY:";
const std::string Station::STATION_API_KEY = 
    "Z276E3rCeTzOQEoBPPN4JCEc6GfvdnYE";

//Station constructor
Station::Station(
    const std::string& name, 
    const std::string& stopID, 
    const std::unordered_set<Train, NSThash>* train_types): 
    name(name), stopID(stopID), update_time(0), update_ftime(""), 
    train_types(train_types) {
        
    nearby.reserve(constant::STATION_RESERVE_NEARBY);
}

// =============================================================================

int Station::update() {
    nearby.clear();

    // headers in get request
    const std::vector<std::string> headers{ 
        "apikey: " + STATION_API_KEY,
        "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/avif,image/webp,*/*;q=0.8"
    };
    std::string data = "";

    if (get_page::get_page(STATION_URL + stopID, headers, data))
        common::panic("curl"); 
    
    update_time = time(nullptr);
    update_ftime = common::formatTime(&update_time, common::SQLITE);

    // parse xml data
    pugi::xml_document doc;
    if (!(doc.load_buffer_inplace(data.data(), data.size())))
        common::panic("doc.load_buffer_inplace");

    // update vector nearby with train info
    populateNearby(doc); 

    return 0;
} 

// populates nearby vector with info on nearby trains using an XML file in local dir
// doc can't be passed as a const otherwise we can't traverse the tree
int Station::populateNearby(pugi::xml_document& doc) {
    pugi::xml_node root = doc.child("LinkedValues");

    // for each type of train (ex. Manhattan-bound F)
    for (auto& incoming_train_type : root.child("item").child("groups").children("groups")) {

        // get info about the type of train, panic if there is a parsing error
        std::string name, headsign;
        int dirID = -1;
        try {
            name = incoming_train_type.child("route").child("id").child("id").text().as_string();
            dirID = incoming_train_type.child("times").child("times").child("directionId").text().as_int();
            headsign = incoming_train_type.child("headsign").text().as_string();
        } catch (const std::exception& e) { common::panic("xml parse error"); }


        // swap name with name on MTA map if necessary
        if (constant::SHUTTLE_NAMES.find(name) != constant::SHUTTLE_NAMES.end()) //if name exists
            name = constant::SHUTTLE_NAMES.at(name); //swap name (assuming equal to ID) with actual name

        // remove special express indicator
        if (name[name.size() - 1] == 'X') 
            name = name.substr(0, name.size() - 1);

        // match train type to known train pointer
        const Train* trainptr = &(*( train_types->find(Train(name, dirID)) )); //pointer to train type being checked
        if ( trainptr == &*(train_types->end()) ) 
            common::panic(name + " " + std::to_string(dirID));

        //check incomings of this type and add to nearby vector
        for (auto& incoming_train : incoming_train_type.child("times").children("times")) { 

            // get the unix time of the current date
            // service day should be a nonnegative int
            time_t service_day = incoming_train.child("serviceDay").text().as_int();
            if (service_day < 0)
                common::panic("serviceDay: " + std::to_string(service_day));

            // get the unix time of the minutes until arrival
            // arrival_time should be a nonnegative int
            // sometimes the realtimeArrival field gives -1 -- in that case, use scheduledArrival
            // if scheduled arrival is also negative, then use realtimedeparture and so on
            time_t arrival_time = incoming_train.child("realtimeArrival").text().as_int();
            if (arrival_time < 0)
                arrival_time = incoming_train.child("scheduledArrival").text().as_int();
            if (arrival_time < 0)
                arrival_time = incoming_train.child("realtimeDeparture").text().as_int();
            if (arrival_time < 0)
                arrival_time = incoming_train.child("scheduledDeparture").text().as_int();
            // at this point, if it is still zero, then it means that the train is just
            // about to arrive/depart

            time_t time = service_day + arrival_time;

            // add this train into the nearby vector
            // sometimes there are duplicate trains (same realtime arrival, type,
            // and direction) but that is fine
            nearby.emplace_back(
                trainptr, headsign, time, common::formatTime(&time, common::SQLITE)
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
