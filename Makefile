.PHONY: all clean

CXX := g++
CXXFLAGS := -std=c++17 -O2
LDFLAGS := -L ./src/curl/lib/.libs -l curl # libcurl
LDFLAGS += -L ./src/sqlite -l sqlite3 # sqlite3

TARGETPATH := ./build/
TARGET := ${TARGETPATH}nyc-subway-tracker.exe

MYOBJECTS := main.o tracker.o tracker_sqlite.o subway.o line.o station.o common.o
OBJECTS := ${MYOBJECTS} pugixml.o


#adjust vars to reflect OS
ifneq (,$(filter ${shell uname}, Linux Darwin)) #Linux or macOS, WIP
TARGET := nyc-subway-tracker
endif


all: ${TARGET}

clean: 
	-rm *.o
	-rm *.stackdump
	-rm ${TARGET}

${TARGET}: ${OBJECTS}
	${CXX} -v ${CXXFLAGS} $^ ${LDFLAGS} -o $@

${MYOBJECTS}: %.o: src/%.cpp
	${CXX} -c ${CXXFLAGS} $< -o $@

pugixml.o: src/pugixml/src/pugixml.cpp # compile pugixml
	${CXX} -c ${CXXFLAGS} $< -o $@

# # nlohmann's json does not need to be compiled individually


#for debug of station
station.exe: station.o pugixml.o
	${CXX} ${CXXFLAGS} $^ ${LDFLAGS} -o $@

#==============================================================================

ROUTES_TARGET := routes_fix.exe

# for routes_fix for processing routes.json
routes_fix: ${ROUTES_TARGET}

${ROUTES_TARGET}: routes_fix.o
	${CXX} ${CXXFLAGS} $< -o src/misc/$@

routes_fix.o: src/misc/routes_fix.cpp
	${CXX} ${CXXFLAGS} -c $< -o $@