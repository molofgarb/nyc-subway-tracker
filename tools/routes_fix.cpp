#include <iostream>
#include <fstream>

#include <string>
#include <vector>

#include <algorithm>

// fixes routes.json file lacking a key before each stop
int main() {

    // filter out non-subway entries
    std::vector<std::string> keepchars{"[", "]", "{", "}"};

    std::ifstream routesFile("resources/routes.json");
    std::ofstream fixFile("temp_routes.json");
    std::string currLine = "";

    while (std::getline(routesFile, currLine)) {
        // std::cout << currLine << std::endl;
        if ( //if syntax symbol or subway
            (std::find(keepchars.begin(), keepchars.end(), currLine.substr(0, 1)) != keepchars.end()) ||
            (currLine.substr(5, 12) == "id\":\"MTASBWY")) {
            fixFile << currLine << std::endl;
            if ((currLine.size() > 6)) { // get the next two lines if subway
                std::getline(routesFile, currLine);
                fixFile << currLine << std::endl;
                std::getline(routesFile, currLine);
                currLine.pop_back();
                fixFile << currLine << std::endl;
            }
        }    
    }
    routesFile.close();
    fixFile.close();


    // filter out blank entries
    std::ifstream inFile("temp_routes.json");
    std::ofstream outFile("new_routes.json");

    currLine = "";
    std::string nextLine = "";
    bool comma = false;

    std::getline(inFile, currLine);

    while (std::getline(inFile, nextLine)) {
        if ((currLine.substr(0, 1) == "{") && (nextLine.substr(0, 1) == "}")) {
            std::getline(inFile, currLine);
        }
        else {// not empty
            if (currLine == "{") { 
                outFile << (comma ? ",\n" : "") << currLine << std::endl;
                comma = true; // do nothing for first {
            }
            else
                //if } then }, otherwise if data then pass data and line break
                outFile << ((currLine.substr(0, 1) == "}") ? "}" : (currLine + '\n'));
            currLine = nextLine;
        }
    }
    outFile << "\n" << currLine << std::endl;

    inFile.close();
    outFile.close();

    std::remove("temp_routes.json");
    // std::cout << "done";
}