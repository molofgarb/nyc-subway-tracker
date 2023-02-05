#ifndef STATION_H
#define STATION_H

#include <iostream>

#include <vector>
#include <string>

class Station {

    struct Train{
        Train(char name, bool express);
        char name;
        bool express;
    };

public:
    Station(std::string& name);

    void update(); //updates nearby trains

    std::string getName();
private:
    //outputs nearby trains
    friend std::ostream& operator<<(std::ostream& os, const Station& rhs);

    std::string name;
    std::vector<Station::Train*> nearby;
};

#endif