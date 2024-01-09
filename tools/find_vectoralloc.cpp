#include <common.h>

// nyc-subway-tracker includes
#include <tracker.h>
#include <subway.h>
#include <line.h>
#include <station.h>

int main(int argc, char* argv[]) {
    int totalNearby = 0;
    int totalStations = 0;
    int totalLines = 0;
    std::vector<int> nearbys;
    std::vector<int> stations;
    Subway subway;
    subway.update();
    for (const auto& stationpair : subway.getStations()) {
        totalNearby += stationpair.second->getNearby().size();
        nearbys.emplace_back(stationpair.second->getNearby().size());

        totalStations++;
    }
    for (const auto& line : subway.getLines()) {
        stations.emplace_back(line.getStations().size());

        totalLines++;
    }
    std::sort(nearbys.begin(), nearbys.end());
    std::sort(stations.begin(), stations.end());
    printf("There are %d nearby\n", totalNearby);
    printf("There are %d stations\n", totalStations);
    printf("There are %d lines\n", totalLines);
    printf("There are on average %f nearby per station\n", float(totalNearby) / float(totalStations));
    printf("There are on average %f stations per line\n", float(totalStations) / float(totalLines));
    printf("There are on median %d nearby per station\n", nearbys[nearbys.size() / 2]);
    printf("There are on median %d stations per line\n", stations[stations.size() / 2]);

    return 0;
}