#ifndef STATION_H
#define STATION_H

#include <iostream>

#include <utility>
#include <vector>
#include <string>
#include <ctime>


struct Train{
    Train(const std::string& name, int dirID): 
        name(name), dirID(dirID) {}
        
    std::string getName() const {return name;}
    int getdirID() const {return dirID;}

    std::string name; //same as name member in line
    int dirID; // 0 for north, 1 for south
};

bool operator==(const Train& lhs, const Train& rhs);
bool operator<(const Train& lhs, const Train& rhs); //(needed for std::map)

class Station {
public:
    Station(const std::string& name, const std::string& stopID, const std::map<Train*, int>* trainTypes);

    void update(); //updates nearby trains

    std::pair<std::string, std::string> getNameAndID() const;
    time_t getTime() const;

    // void debug(); //not intended for normal use

private:
    //outputs nearby trains
    friend std::ostream& operator<<(std::ostream& os, const Station& rhs);

    //used for update()
    void getStationXML();
    void populateNearby();

    std::string name;
    std::string stopID;

    std::vector<std::pair<const Train*, int>> nearby; //int is time in unix time
    std::time_t updateTime; //unix timestamp when nearby was updated

    const std::map<Train*, int>* trainTypes;
};

#endif