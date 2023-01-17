#ifndef SUBWAY_H
#define SUBWAY_H

#include <memory>

#include <iostream>

#include <string>
#include <vector>
#include <map>

#include "station.h"
#include "line.h"

using st_ptr = std::shared_ptr<Station>;


class Subway {
public:
    Subway();
    Subway(const Subway& other);
    Subway& operator=(const Subway& other);
    ~Subway();

    int update();

    const std::vector<Line*>& getLines() const {return lines;}
    const std::map<std::string, st_ptr>* getStations() const {return allStations;}
    
    std::ostream& outputByStation(std::ostream& os) const;
    std::ostream& outputByLine(std::ostream& os, bool allowRepeat=true) const;

    // void debug(const std::string& in); //not intended for normal use

private:
    friend std::ostream& operator<<(std::ostream& os, const Subway& rhs);

    //used in constructor
    int parseSubwayJSON(std::string& jsonData);

    std::vector<Line*> lines;
    std::map<Train*, int>* trainTypes; //train, dummy int
    std::map<std::string, st_ptr>* allStations; //stationID, ptr to station

    //subway should usually update the stations instead of line
};

#endif