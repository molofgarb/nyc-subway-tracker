.PHONY: all clean

CC := g++
FLAGS := -std=c++17
LINKS := -L .\src\curl\lib\.libs -l libcurl
# LINKS := -I .\src\curl\lib\.libs\*.a -I .\src\curl\lib\.libs\*.o -l libcurl
# LINKS := -L C:\Users\Nathan\Documents\!Repos\nyc-subway-tracker\src\curl\lib\.libs -l libcurl
# links to libcurl library

TARGET := main.exe


#adjust vars to reflect OS
ifneq (,$(filter $(uname), Linux Darwin)) #Linux or macOS, WIP
TARGET := main
ROUTES_TARGET := routes_fix
endif


all: main.exe

clean: 
	-rm *.o

#WIP
${TARGET}: main.o station.o pugixml.o json.o
	${CC} ${FLAGS} $^ ${LINKS} -o $@

main.o: src/main.cpp 
	${CC} -c ${FLAGS} $^ -o $@

station.o: src/station.cpp
	${CC} -c ${FLAGS} $^ -o $@

pugixml.o: src/pugixml/src/pugixml.cpp # compile pugixml
	${CC} -c ${FLAGS} $^ -o $@

json.o: src/nlohmann/single_include/nlohmann/json.hpp # compile json
	${CC} -c ${FLAGS} -x c++ $^ -o $@



#for debug of station
station.exe: station.o pugixml.o
	${CC} ${FLAGS} $^ ${LINKS} -o $@

#==============================================================================

ROUTES_TARGET := routes_fix.exe

# for routes_fix for processing routes.json
routes_fix: ${ROUTES_TARGET}

${ROUTES_TARGET}: routes_fix.o
	${CC} ${FLAGS} $^ -o src/misc/$@

routes_fix.o: src/misc/routes_fix.cpp
	${CC} ${FLAGS} -c $^ -o $@