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
    std::string name;
    int dirID; // 0 for north, 1 for south
};

inline bool operator==(const Train& lhs, const Train& rhs);

class Station {
public:
    Station(const std::string& name, const std::string& stopID, const std::vector<Train*>* trainTypes);

    void update(); //updates nearby trains

    std::pair<std::string, std::string> getNameAndID() const;
    time_t getTime() const;
private:
    //outputs nearby trains
    friend std::ostream& operator<<(std::ostream& os, const Station& rhs);

    //used for update()
    friend void getStationXML(Station& station);
    friend void populateNearby(Station& station);

    std::string name;
    std::string stopID;

    std::vector<std::pair<Train*, int>> nearby; //int is time in unix time
    std::time_t updateTime; //unix timestamp when nearby was updated

    const std::vector<Train*>* trainTypes;
};

#endif