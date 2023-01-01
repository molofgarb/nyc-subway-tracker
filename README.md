# nyc-subway-tracker ![MIT](https://img.shields.io/badge/license-MIT-blue.svg)
A program that logs the history of the subway train arrival time predictions from https://new.mta.info/.
This program can be used to predict the delay between any train service for any station at any time.

## Installation
Please see the releases section on the right side of the page to download an official release. The program
can be self-built using the provided Makefile.

## Usage
work in progress

## Notes 
work in progress

## Why?
The data from this tool can be used to find the time between trains for a station that you are planning to visit.
If the data shows that the station has a long delay for a train service that you need (e.g. 15 min delay), you can
know to be more proactive with checking the subway status to avoid being stuck in the station for a while.

## Credits
JosephP91 for curlcpp to use as a wrapper for curl
nhohmann for their JSON for Modern C++ project for providing a library to parse .json files.
zeux for their pugixml project for providing a library to parse .xml files.
The SQLite3 team for the SQLite3 project to use for working with an SQLite3 database.
The MTA for providing real-time subway arrival times.

[curlcpp](https://github.com/JosephP91/curlcpp)
[JSON for Modern C++](https://github.com/nlohmann/json)
[pugixml](https://github.com/zeux/pugixml)
[SQLite3](https://www.sqlite.org/index.html)
[MTA](https://new.mta.info/)