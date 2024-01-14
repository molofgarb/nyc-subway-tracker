#ifndef STATION_H
#define STATION_H

#include <pch.h>

// external includes
#include <pugixml.hpp>

class Station {
public:
    Station(
        const std::string& name, 
        const std::string& stopID, 
        const std::unordered_set<Train, NSThash>* trainTypes);

    int update(); // updates nearby trains

    const std::string& getName() const {return name;}
    const std::string& getID() const {return stopID;}
    const std::vector<Arrival>& getNearby() const {return nearby;}
    std::time_t getTime() const {return update_time;}
    std::string getfTime() const {return update_ftime;}

    // void debug(); //not intended for normal use

private:
    // outputs nearby trains
    friend std::ostream& operator<<(std::ostream& os, const Station& rhs);

    // used for update()
    int populateNearby(pugi::xml_document& doc);

    std::string name;
    std::string stopID;

    std::vector<Arrival> nearby; 
    std::time_t update_time; // unix timestamp when nearby was updated
    std::string update_ftime;

    const std::unordered_set<Train, NSThash>* train_types;

    static const std::string STATION_URL;
    static const std::string STATION_API_KEY;
};

#endif