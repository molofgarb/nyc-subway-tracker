#ifndef LINE_H
#define LINE_H

#include <memory>

#include <iostream>

#include <vector>
#include <string>

#include "station.h"

using st_ptr = std::shared_ptr<Station>;


class Line {
public:
    Line(const std::string& name,
         const std::string& ID, 
         const std::map<Train*, int>* trainTypes,
         std::map<std::string, st_ptr>* allStations
    );
    // Heap memory by Subway (for now)
    Line(const Line& other);
    Line& operator=(const Line& other);
    ~Line();

    //updates the stations throughout the line (updates nearby for each station)
    int update(); 

    //returns stations (used in subway)
    const std::vector<st_ptr>& getStations() const {return stations;}

    const std::string& getName() const {return name;}
    const std::string& getID() const {return ID;}

private:
    friend std::ostream& operator<<(std::ostream& os, const Line& rhs);

    //used in constructor
    int parseLineJSON(std::string& jsonData, 
                       const std::map<Train*, int>* trainTypes, 
                       std::map<std::string, st_ptr>* allStations
    );

    std::string name; //name as shown on subway map
    std::string ID; //id used in getting webpage; usually same as name, with exceptions
    std::vector<st_ptr> stations;

    const std::map<Train*, int>* trainTypes;
};

#endif