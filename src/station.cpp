#include <iostream>

#include <vector>
#include <string>

#include <stdio.h>
#include <curl/curl.h>

#include "pugixml/src/pugixml.hpp"

#include "station.h"

const std::string STATION_API_KEY = "Z276E3rCeTzOQEoBPPN4JCEc6GfvdnYE";
// curl -v -H "apikey: Z276E3rCeTzOQEoBPPN4JCEc6GfvdnYE" https://otp-mta-prod.camsys-apps.com/otp/routers/default/nearby?stops=MTASBWY%3AG14


// from libcurl url2file (https://curl.se/libcurl/c/url2file.html)
static size_t write_data(void *ptr, size_t size, size_t nmemb, void *stream) {
  size_t written = fwrite(ptr, size, nmemb, (FILE*)stream);
  return written;
}

//Station constructor
Station::Station(const std::string& name, const std::string& stopID, std::vector<Train*>* trainTypes): 
    name(name), stopID(stopID), trainTypes(trainTypes) {
    this->update();
}

//gets the XML file from the MTA with info about arriving trains
void getStationXML(Station& station) {
    // go to the URL associated with the stopID
    std::string url = ("https://otp-mta-prod.camsys-apps.com/otp/routers/default/nearby?stops=MTASBWY%3A" + station.stopID);
    const char* outFile = "stationTemp.xml";
    
    CURL* curl = curl_easy_init();
    FILE* file;
    CURLcode res;
    
    curl_global_init(CURL_GLOBAL_DEFAULT);

    //configure headers
    struct curl_slist* headers = NULL;
    std::string apikeyHeader = "apikey: " + STATION_API_KEY;
    std::string contentType = "Content-Type: application/xml";
    std::string accept = "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/avif,image/webp,*/*;q=0.8";
    headers = curl_slist_append(headers, apikeyHeader.data());
    headers = curl_slist_append(headers, contentType.data());
    headers = curl_slist_append(headers, accept.data());

    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url.data());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
        curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
        curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1L);
        file = fopen(outFile, "wb");
        if (file) {
            std::cout << curl << std::endl;
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, file);
            res = curl_easy_perform(curl);
            std::cout << "CURLcode: " << res << std::endl;
            fclose(file);
        }
    }
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
    curl_global_cleanup();
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
    std::remove("stationTemp.xml"); // remove XML file downloaded
} 

std::string Station::getName() const {
    return name;
}


std::ostream& operator<<(std::ostream& os, const Station& rhs) {
    os << "These are the nearby trains:" << std::endl;
    for (std::pair<Train*, int> arrival : rhs.nearby) {
        os << "\t";
        os << arrival.first->name << ":\n\t\t" << "Time: " << arrival.second << std::endl;
    }
    return os;
}