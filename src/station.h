#ifndef STATION_H
#define STATION_H

#include <iostream>

#include <utility>
#include <vector>
#include <string>

struct Train{
    Train(std::string& name, int dirID): 
        name(name), dirID(dirID) {}
    std::string name;
    int dirID; // 0 for north, 1 for south
};

inline bool operator==(const Train& lhs, const Train& rhs) {
    return ((lhs.name == rhs.name) && (lhs.dirID == rhs.dirID));
}

class Station {
public:
    Station(const std::string& name, const std::string& stopID, std::vector<Train*>* trainTypes);

    void update(); //updates nearby trains

    std::string getName() const;
private:
    //outputs nearby trains
    friend std::ostream& operator<<(std::ostream& os, const Station& rhs);
    friend void getStationXML(Station& station);
    friend void populateNearby(Station& station);

    std::string name;
    std::string stopID;
    std::vector<std::pair<Train*, int>> nearby; //int is time in unix time

    std::vector<Train*>* trainTypes;
};

#endif