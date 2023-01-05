#ifndef LINE_H
#define LINE_H

#include <iostream>

#include <vector>
#include <string>

#include "station.h"

class Line {
public:
    Line(const std::string& name,
         const std::string& ID, 
         const std::map<Train*, int>* trainTypes,
         std::map<std::string, Station*>* allStations
    );
    // Heap memory by Subway (for now)
    Line(const Line& other);
    Line& operator=(const Line& other);
    ~Line();

    //updates the stations throughout the line (updates nearby for each station)
    void update(); 

    //returns stations (used in subway)
    const std::vector<Station*>* getStations() const;

    std::string getName() const;
    std::string getID() const;

private:
    friend std::ostream& operator<<(std::ostream& os, const Line& rhs);

    //used in constructor
    void parseLineJSON(const std::string& filename, 
                       const std::map<Train*, int>* trainTypes, 
                       std::map<std::string, Station*>* allStations
    );

    std::string name; //name as shown on subway map
    std::string ID; //id used in getting webpage; usually same as name, with exceptions
    std::vector<Station*> stations;

    const std::map<Train*, int>* trainTypes;
};

#endif