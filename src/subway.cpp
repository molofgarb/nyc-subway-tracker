#include <iostream>
#include <fstream>

#include <string>
#include <vector>
#include <map>

#include <ctime>

#include "nlohmann/single_include/nlohmann/json.hpp"

#include "getPage.h"
#include "station.h"
#include "line.h"

#include "subway.h"


Subway::Subway() {
    trainTypes = new std::map<Train*, int>();
    allStations = new std::map<std::string, Station*>();

    const std::vector<std::string> headers{
        "apikey: " + constant::SUBWAY_API_KEY,
    };
    const std::string outFile = "subwayTemp.json";

    get_page::getPage(constant::SUBWAY_URL, headers, outFile);
    parseSubwayJSON(outFile);
    std::remove(outFile.data());
}

// fills out lines and trainTypes with data from subwayTemp.json
void Subway::parseSubwayJSON(const std::string& filename) {
    std::ifstream file(filename);
    nlohmann::json data = nlohmann::json::parse(file);

    for (nlohmann::json item : data) { //create trainTypes and lines
        auto originalID = item["id"].get<std::string>();
        auto originalName = (item["shortName"].is_null()) ? 
                                "" : item["shortName"].get<std::string>();

        //regular subway and not special express type of subway
        if ((originalID.substr(0, 8) == "MTASBWY:") && (originalID[originalID.size() - 1] != 'X')) { 
            std::string newID = std::string(
                originalID.substr(8, int(originalID.size()) - 8)); //all characters after ":"
            std::string newName = (originalName == "S") ?
                constant::SHUTTLE_NAMES.at(newID) : //if subway shuttle
                originalName; //if regular subway

            (*trainTypes)[new Train(newName, 0)] = 0; //add a train going in each direction
            (*trainTypes)[new Train(newName, 1)] = 0;

            Line* line = new Line(newName, newID, trainTypes, allStations);
            lines.push_back(line);
        }
    }
}

Subway::Subway(const Subway& other) {
    for (Line* line : other.lines) {
        lines.push_back(new Line(*line));
    }
    trainTypes = new std::map<Train*, int>(*other.trainTypes);
    allStations = new std::map<std::string, Station*>(*other.allStations);
}

Subway& Subway::operator=(const Subway& other) {
    if (this != &other) {
        for (Line* line : other.lines) {
            lines.push_back(new Line(*line));
        }
        trainTypes = new std::map<Train*, int>(*other.trainTypes);
        allStations = new std::map<std::string, Station*>(*other.allStations);
    }
    return *this;
}

Subway::~Subway() {
    for (Line* line : lines) { //destroy lines
        if (line != nullptr) {
            delete line;
            line = nullptr;
        }
    }
    lines.clear();

    for (std::pair<Train*, int> train : *trainTypes) { //destroy trainTypes
        if (train.first != nullptr) {
            delete train.first;
            train.first = nullptr;
        }
    }
    trainTypes->clear();
    delete trainTypes;
    trainTypes = nullptr;

    for (std::pair<std::string, Station*> stationPair : *allStations) {
        if (stationPair.second != nullptr) {
            delete stationPair.second;
            stationPair.second = nullptr;
        }
    }
    allStations->clear();
    delete allStations;
    allStations = nullptr;
}

//update each station from stations
void Subway::update() {
    for (std::pair<std::string, Station*> stationPair : *allStations) {
        stationPair.second->update();
    }
    // (*allStations)["G14"]->update(); .// <DEBUG>
}

std::ostream& Subway::output(std::ostream& os, bool allowRepeat) const {
    if (allowRepeat) {
        for (const Line* line : lines) {
            os << *line << '\n';
        }
    } else { // do not repeat stations
        // WIP
    }
    return os;
}

std::ostream& Subway::outputByStation(std::ostream& os) const {
    os << "These are the nearby trains for all stations." << '\n';

    char timestr[128] = "";
    const auto now = std::time(nullptr);

    if (std::strftime(timestr, sizeof(timestr), "%I:%M:%S %p", std::localtime(&now)))
        os << "The current time is " << timestr << '.' << '\n';

    os << "==================================================" << '\n';
    for (std::pair<std::string, Station*> stationPair : *allStations) {
        os << *(stationPair.second) << '\n';
    }
    return os << '\n';
}

std::ostream& operator<<(std::ostream& os, const Subway& rhs) {
    return rhs.outputByStation(os);
}

// ===== DEBUG ================================================================

// void Subway::debug(const std::string& in) {
//     std::cout << "In: " << in << '\n';
//     std::cout << "allStations Station addr: " << (*allStations)[in] << '\n';
//     std::cout << "allStations Station name: " << (*allStations)[in]->getNameAndID().first << '\n';
//     std::cout << "allStations Station ID: " << (*allStations)[in]->getNameAndID().second << '\n';
//     for (std::pair<Train*, int> type : *trainTypes) {
//         std::cout << "\tTrain: " << type.first << ' ' << type.first->getName() << '\n';
//     }
//     std::cout << "debugging Station now..." << '\n';
//     (*allStations)[in]->debug();
// }