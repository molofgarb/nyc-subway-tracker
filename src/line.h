#ifndef LINE_H
#define LINE_H

#include <iostream>

#include <vector>
#include <string>

#include "station.h"

class Line {
public:
    Line(const std::string& name, const std::vector<Train*>* trainTypes);
    Line(const Line& other);
    Line& operator=(const Line& other);
    ~Line();

    //updates the stations throughout the line (updates nearby for each station)
    void update(); 
private:
    friend std::ostream& operator<<(std::ostream& os, const Line& rhs);

    //used in consturctor
    friend void parseLineJSON(Line& line, const std::string& filename, const std::vector<Train*>* trainTypes);

    std::string name;
    std::vector<Station*> stations;

    const std::vector<Train*>* trainTypes;
};

#endif