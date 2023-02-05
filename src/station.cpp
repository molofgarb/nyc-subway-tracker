#include <iostream>

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
        std::cout << "error code: 1000" << std::endl; //unable to open
        return;
    }

    pugi::xml_node root = doc.child("LinkedValues");

    for (pugi::xml_node incomingTrainType: // for each type of train (ex. Manhattan-bound F)
            root.child("item").child("groups").children("groups") 
            ) {
        // get info about the type of train
        std::string name = incomingTrainType.child("route").child("id").child("id").text().as_string();
        int dirID = incomingTrainType.child("times").child("times").child("directionId").text().as_int();

        if (constant::SHUTTLE_NAMES.find(name) != constant::SHUTTLE_NAMES.end()) //if name exists
            name = constant::SHUTTLE_NAMES.at(name); //swap name (assuming equal to ID) with actual name

        if (name[name.size() - 1] == 'X')
            name = name.substr(0, name.size() - 1); //get rid of X in front of name (X not used in traintypes)

        // match train type to known train pointer
        const Train* trainptr = nullptr; //pointer to train type being checked
        for (const std::pair<Train*, int>& trainPair : *trainTypes) { //find the pointer to the matching train
            const Train* trainType = std::get<0>(trainPair); //pointer to Train
            if ((name == trainType->name) && (dirID == trainType->dirID)) {
                trainptr = trainType; //set train to point to defined train type
                break; 
            }
        }
        if (trainptr == nullptr) { //debug stuff
            std::cout << "error <station.cpp>: 2000 " + name + " " << 
                         dirID << std::endl; // error
        }

        //check incomings of this type and add to nearby vector
        for (pugi::xml_node incomingTrain: incomingTrainType.child("times").children("times")) { 
            int time = //in unix time
                    incomingTrain.child("serviceDay").text().as_int() +
                    incomingTrain.child("realtimeArrival").text().as_int(); // realtime is the number used on webpage
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
    os << "These are the nearby trains for " << rhs.name << ": " << std::endl;
    for (std::pair<const Train*, int> arrival : rhs.nearby) {
        std::time_t dateTime(arrival.second);
        os << arrival.first->name << 
            ", " << ((arrival.first->dirID > 0) ?  "southbound" : "northbound") <<
            ":\t" << "Unix Timestamp: " << arrival.second << 
            "\t" << ctime(&dateTime);
    }
    return os;
}

// ===== DEBUG ================================================================

// void Station::debug() {
//     std::cout << "My name is " << name << std::endl;
//     std::cout << "My ID is " << stopID << std::endl;
//     std::cout << "My updateTime is " << updateTime << std::endl;
//     std::cout << "Nearbys: " << nearby.size() << std::endl;
//     for (std::pair<Train*, int> type : *trainTypes) {
//         std::cout << "\tTrain: " << type.first << ' ' << type.first->getName() << std::endl;
//     }
//     std::cout << "\n\n" << std::endl;
//     for (std::pair<Train*, int> type : *trainTypes) {
//         std::cout << "\tTrain: " << type.first << ' ' << type.first->getName() << std::endl;
//     }
//     for (std::pair<const Train*, int> arrival : nearby) {
//         std::cout << "\tTrain: " << arrival.first->name << std::endl;
//         std::cout << "\tdirID: " << arrival.first->dirID << std::endl;
//         std::cout << "\tTime: " << arrival.second << std::endl;
//     }
// }