#include <iostream>

#include <utility>
#include <vector>
#include <string>

#include <ctime>

#include "pugixml/src/pugixml.hpp"

#include "getPage.h"

#include "station.h"


//Train equality operator
bool operator==(const Train& lhs, const Train& rhs) {
    return ((lhs.name == rhs.name) && (lhs.dirID == rhs.dirID));
}

//Train comparison operator (needed for std::map)
bool operator<(const Train& lhs, const Train& rhs) {
    return (lhs.name == rhs.name) ? (lhs.dirID < rhs.dirID) : (lhs.name < rhs.name);
}

// ===== STATION ==============================================================

//Station constructor
Station::Station(const std::string& name, const std::string& stopID, const std::map<Train*, int>* trainTypes): 
    name(name), stopID(stopID), updateTime(0), trainTypes(trainTypes) {}

//gets the XML file from the MTA with info about arriving trains
void Station::getStationXML() {
    const std::string url = constant::STATION_URL + stopID; //url to web page with data
    const std::vector<std::string> headers{ //headers in get request
        "apikey: " + constant::STATION_API_KEY,
        "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/avif,image/webp,*/*;q=0.8"
    };
    const std::string outFile = "stationTemp.xml"; //file created to hold xml downloaded
    
    get_page::getPage(url, headers, outFile);
}

//populates nearby vector with info on nearby trains using an XML file in local dir
void Station::populateNearby() {
    //parsing XML file
    pugi::xml_document doc;
    pugi::xml_parse_result result = doc.load_file("stationTemp.xml");
    if (!result) {
        std::cout << "error code: 1000" << '\n'; //unable to open
        return;
    }

    pugi::xml_node root = doc.child("LinkedValues");

    for (auto incomingTrainType: // for each type of train (ex. Manhattan-bound F)
            root.child("item").child("groups").children("groups") 
            ) {
        // get info about the type of train
        std::string name = incomingTrainType.child("route").child("id").child("id").text().as_string();
        int dirID = incomingTrainType.child("times").child("times").child("directionId").text().as_int();

        //swap name with name on MTA map if necessary
        if (constant::SHUTTLE_NAMES.find(name) != constant::SHUTTLE_NAMES.end()) //if name exists
            name = constant::SHUTTLE_NAMES.at(name); //swap name (assuming equal to ID) with actual name

        if (name[name.size() - 1] == 'X') //remove special express indicator
            name = name.substr(0, name.size() - 1);

        // match train type to known train pointer
        const Train* trainptr = nullptr; //pointer to train type being checked
        for (const auto& trainPair : *trainTypes) { //find the pointer to the matching train
            const Train* trainType = std::get<0>(trainPair); //pointer to Train
            if ((name == trainType->name) && (dirID == trainType->dirID)) {
                trainptr = trainType; //set train to point to defined train type
                break; 
            }
        }
        if (trainptr == nullptr) { //<DEBUG>
            std::cout << "error <station.cpp>: 2000 " + name + " " << 
                         dirID << '\n'; // error
        }

        //check incomings of this type and add to nearby vector
        for (auto incomingTrain: incomingTrainType.child("times").children("times")) { 
            time_t time = //in unix time, realtime is the number used on webpage
                    incomingTrain.child("serviceDay").text().as_int() +
                    incomingTrain.child("realtimeArrival").text().as_int(); 
            nearby.push_back(std::make_pair(trainptr, time));
        }
    }

}

void Station::update() {
    nearby.clear();
    getStationXML(); //get XML file with train info
    populateNearby(); //update vector nearby with train info
    updateTime = time(nullptr); //timestamp when object fully updated
    std::remove("stationTemp.xml"); // remove XML file downloaded
} 

std::pair<std::string, std::string> Station::getNameAndID() const {
    return std::pair(name, stopID);
}

std::time_t Station::getTime() const {
    return updateTime;   
}

std::ostream& operator<<(std::ostream& os, const Station& rhs) {
    std::time_t timeNow = std::time(nullptr);
    char timestr[128] = "";

    os << "The current station is " << rhs.name << ", " << rhs.stopID << '.' << '\n';

    if (std::strftime(timestr, sizeof(timestr), "%I:%M:%S %p", std::localtime(&timeNow)))
        os << "The current time is " << timestr << '.' << '\n';
    if (std::strftime(timestr, sizeof(timestr), "%I:%M:%S %p", std::localtime(&rhs.updateTime)))
        os << "The data was updated for this station at " << timestr << '.' << '\n';

    for (auto arrival : rhs.nearby) {
        std::time_t untilArrivalTime = arrival.second - timeNow; //diff arrival to now time
        std::time_t arrivalTime(int(arrival.second) % (86400)); //updateTime in sec from start of day

        os << arrival.first->name << ", " << 
            ((arrival.first->dirID > 0) ?  "southbound (1)" : "northbound (0)");
        if (std::strftime(timestr, sizeof(timestr), "%M:%S", std::localtime(&untilArrivalTime)))
            os << ",\t" << "Time until arrival from now: " << timestr;
        if (std::strftime(timestr, sizeof(timestr), "%I:%M:%S %p", std::localtime(&arrivalTime)))
            os << ",\t" << "Arrival Time: " << timestr;
        os << '\n';

    }
    return os << '\n';
}


// ===== DEBUG ================================================================

// void Station::debug() {
//     std::cout << "My name is " << name << '\n';
//     std::cout << "My ID is " << stopID << '\n';
//     std::cout << "My updateTime is " << updateTime << '\n';
//     std::cout << "Nearbys: " << nearby.size() << '\n';
//     for (std::pair<Train*, int> type : *trainTypes) {
//         std::cout << "\tTrain: " << type.first << ' ' << type.first->getName() << '\n';
//     }
//     std::cout << "\n\n" << '\n';
//     for (std::pair<Train*, int> type : *trainTypes) {
//         std::cout << "\tTrain: " << type.first << ' ' << type.first->getName() << '\n';
//     }
//     for (std::pair<const Train*, int> arrival : nearby) {
//         std::cout << "\tTrain: " << arrival.first->name << '\n';
//         std::cout << "\tdirID: " << arrival.first->dirID << '\n';
//         std::cout << "\tTime: " << arrival.second << '\n';
//     }
// }