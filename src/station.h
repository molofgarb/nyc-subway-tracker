#ifndef STATION_H
#define STATION_H

#include <iostream>

#include <utility>
#include <vector>
#include <string>

#include <ctime>

#include "pugixml/src/pugixml.hpp"


struct Train {
    Train(const std::string& name, int dirID);
        
    std::string getName() const {return name;}
    int getdirID() const {return dirID;}

    std::string name; //same as name member in line
    int dirID; // 0 for north, 1 for south
};
bool operator==(const Train& lhs, const Train& rhs);
bool operator<(const Train& lhs, const Train& rhs); //(needed for std::map)

struct Arrival {
    Arrival(const Train* train=nullptr, std::time_t time=0);

    const Train* getTrain() const {return train;}
    std::time_t get_time() const {return time;}

    const Train* train;
    std::time_t time;
};

class Station {
public:
    Station(const std::string& name, 
            const std::string& stopID, 
            const std::map<Train*, int>* trainTypes);

    void update(); //updates nearby trains

    std::pair<std::string, std::string> getNameAndID() const;
    std::time_t getTime() const;

    // void debug(); //not intended for normal use

private:
    //outputs nearby trains
    friend std::ostream& operator<<(std::ostream& os, const Station& rhs);

    //used for update()
    void populateNearby(pugi::xml_document& doc);

    std::string name;
    std::string stopID;

    std::vector<Arrival> nearby; 
    std::time_t updateTime; //unix timestamp when nearby was updated

    const std::map<Train*, int>* trainTypes;
};

#endif