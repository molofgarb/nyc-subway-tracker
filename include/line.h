#ifndef LINE_H
#define LINE_H

#include <common.h>

// nyc-subway-tracker includes
#include <station.h>

class Line {
public:
    Line(const std::string& name,
         const std::string& ID, 
         std::unordered_map<std::string, st_ptr>& all_stations,
         const std::unordered_set<Train, NSThash>* train_types);

    //updates the stations throughout the line (updates nearby for each station)
    int update(); 

    //returns stations (used in subway)
    const std::vector<st_ptr>& getStations() const {return stations;}

    const std::string& getName() const {return name;}
    const std::string& getID() const {return ID;}

private:
    friend std::ostream& operator<<(std::ostream& os, const Line& rhs);

    //used in constructor
    int parseLineJSON(
        std::string& jsonData, 
        std::unordered_map<std::string, st_ptr>& all_stations);

    std::string name; //name as shown on subway map
    std::string ID; //id used in getting webpage; usually same as name, with exceptions
    std::vector<st_ptr> stations;

    const std::unordered_set<Train, NSThash>* train_types;
};

#endif