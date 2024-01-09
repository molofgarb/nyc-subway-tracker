#ifndef SUBWAY_H
#define SUBWAY_H

#include <common.h>

// nyc-subway-tracker includes
#include <station.h>
#include <line.h>

using st_ptr = std::shared_ptr<Station>;

class Subway {
public:
    Subway();

    int update();

    const std::vector<Line>& getLines() const {return lines;}
    const std::unordered_map<std::string, st_ptr>& getStations() const {return allStations;}
    
    std::ostream& outputByStation(std::ostream& os) const;
    std::ostream& outputByLine(std::ostream& os, bool allowRepeat=true) const;

    // void debug(const std::string& in); //not intended for normal use

private:
    friend std::ostream& operator<<(std::ostream& os, const Subway& rhs);

    //used in constructor
    int parseSubwayJSON(std::string& jsonData);

    int updateThread(size_t offset);  

    std::vector<Line> lines;
    std::unordered_map<std::string, st_ptr> allStations; //stationID, ptr to station
    std::unordered_set<Train, NSThash> trainTypes; //train, dummy int

    //subway should usually update the stations instead of line
};

#endif