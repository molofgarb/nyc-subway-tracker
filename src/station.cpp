#include <iostream>

#include <vector>
#include <string>
#include <ctime>

#include "pugixml/src/pugixml.hpp"

#include "getPage.h"

#include "station.h"

const std::string STATION_URL = "https://otp-mta-prod.camsys-apps.com/otp/routers/default/nearby?stops=MTASBWY%3A";
const std::string STATION_API_KEY = "Z276E3rCeTzOQEoBPPN4JCEc6GfvdnYE";


//Train equality operator
inline bool operator==(const Train& lhs, const Train& rhs) {
    return ((lhs.name == rhs.name) && (lhs.dirID == rhs.dirID));
}

// ===== STATION ==============================================================

//Station constructor
Station::Station(const std::string& name, const std::string& stopID, const std::vector<Train*>* trainTypes): 
    name(name), stopID(stopID), updateTime(0), trainTypes(trainTypes) {}

//gets the XML file from the MTA with info about arriving trains
void getStationXML(Station& station) {
    const std::string url = STATION_URL + station.stopID; //url to web page with data
    const std::vector<std::string> headers{ //headers in get request
        "apikey: " + STATION_API_KEY,
        "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/avif,image/webp,*/*;q=0.8"
    };
    const std::string outFile = "stationTemp.xml"; //file created to hold xml downloaded
    
    get_page::getPage(url, headers, outFile);
}

//populates nearby vector with info on nearby trains using an XML file in local dir
void populateNearby(Station& station) {
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

        // match train type to known train pointer
        Train* trainptr = nullptr; //pointer to train type being checked
        for (Train* trainType: *station.trainTypes) { //find the pointer to the matching train
            // std::cout << "name: " << trainType->name << " and dirID: " << trainType->dirID << std::endl;
            if ((name == trainType->name) && (dirID == trainType->dirID)) {
                trainptr = trainType; //set train to point to defined train type
                break; 
            }
        }
        if (trainptr == nullptr) std::cout << "error code: 2000" << std::endl; // error

        //check incomings of this type and add to nearby vector
        for (pugi::xml_node incomingTrain: incomingTrainType.child("times").children("times")) { 
            int time = //in unix time
                    incomingTrain.child("serviceDay").text().as_int() +
                    incomingTrain.child("realtimeArrival").text().as_int(); // realtime is the number used on webpage
            station.nearby.push_back(std::make_pair(trainptr, time));
        }
    }
}

void Station::update() {
    getStationXML(*this); //get XML file with train info
    populateNearby(*this); //update vector nearby with train info
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
    for (std::pair<Train*, int> arrival : rhs.nearby) {
        std::time_t dateTime(arrival.second);
        os << arrival.first->name << 
            ", " << ((arrival.first->dirID > 0) ?  "southbound" : "northbound") <<
            ":\t" << "Unix Timestamp: " << arrival.second << 
            "\t" << ctime(&dateTime);
    }
    return os;
}