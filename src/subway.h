#ifndef SUBWAY_H
#define SUBWAY_H

#include <iostream>

#include <string>
#include <vector>
#include <map>

#include "station.h"
#include "line.h"


class Subway {
public:
    Subway();
    Subway(const Subway& other);
    Subway& operator=(const Subway& other);
    ~Subway();

    void update();
    
    std::ostream& outputByStation(std::ostream& os) const;

    // void debug(const std::string& in); //not intended for normal use

private:
    friend std::ostream& operator<<(std::ostream& os, const Subway& rhs);

    //used in operator<< overload
    std::ostream& output(std::ostream& os, bool allowRepeat=true) const;

    //used in constructor
    void parseSubwayJSON(const std::string& filename);

    std::vector<Line*> lines;
    std::map<Train*, int>* trainTypes; //train, dummy int
    std::map<std::string, Station*>* allStations; //stationID, ptr to station

    //subway should usually update the stations instead of line
};

#endif