#include <iostream>

#include <vector>
#include <string>

#include "station.h"

Station::Train::Train(char name, bool express): 
    name(name), express(express) {}