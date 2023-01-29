#ifndef STATION_H
#define STATION_H

#include <iostream>

#include <utility>
#include <vector>
#include <string>
#include <map>

#include <ctime>

// external includes
#include <pugixml.hpp>


struct Train {
    Train(const std::string& name, int dirID):
        name(name), dirID(dirID) {}

    std::string name; //same as name member in line
    int dirID; // 0 for north, 1 for south
};
bool operator==(const Train& lhs, const Train& rhs);
bool operator<(const Train& lhs, const Train& rhs); //(needed for std::map)

struct Arrival {
    Arrival(const Train* train=nullptr, std::time_t time=0):
        train(train), time(time) {}

    const Train* train;
    std::time_t time; //time of arrival
};

class Station {
public:
    Station(const std::string& name, 
            const std::string& stopID, 
            const std::map<Train*, int>* trainTypes);

    int update(); //updates nearby trains

    const std::string& getName() const {return name;}
    const std::string& getID() const {return stopID;}
    const std::vector<Arrival>& getNearby() const {return nearby;}
    std::time_t getTime() const {return updateTime;}

    // void debug(); //not intended for normal use

private:
    //outputs nearby trains
    friend std::ostream& operator<<(std::ostream& os, const Station& rhs);

    //used for update()
    int populateNearby(pugi::xml_document& doc);

    std::string name;
    std::string stopID;

    std::vector<Arrival> nearby; 
    std::time_t updateTime; //unix timestamp when nearby was updated

    const std::map<Train*, int>* trainTypes;
};

#endif