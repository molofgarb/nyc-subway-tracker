.PHONY: all clean

CXX := g++
CXXFLAGS := -std=c++17 -O0
LDFLAGS := -L ./src/curl/lib/.libs -l curl # libcurl.a
LDFLAGS += -L ./src/sqlite -l sqlite3 # libsqlite3.a

TARGETPATH := ./
TARGET := ${TARGETPATH}nyc-subway-tracker.exe

MYOBJECTS := subway.o line.o station.o common.o
MYOBJECTS += tracker.o tracker_sqlite.o 
MYOBJECTS += nyc-subway-tracker.o 

OBJECTS := pugixml.o ${MYOBJECTS}


#adjust vars to reflect OS
ifneq (,$(filter ${shell uname}, Linux Darwin)) #Linux or macOS, WIP
TARGET := nyc-subway-tracker
endif


all: ${TARGET}

clean: 
	-rm *.o
	-rm *.stackdump
	-rm ${TARGET}
	-rm subway.txt

${TARGET}: ${OBJECTS}
	-mkdir ${TARGETPATH}
	${CXX} ${CXXFLAGS} $^ ${LDFLAGS} -o $@

${MYOBJECTS}: %.o: src/%.cpp
	${CXX} -c ${CXXFLAGS} $< -o $@

pugixml.o: src/pugixml/src/pugixml.cpp # compile pugixml
	${CXX} -c ${CXXFLAGS} $< -o $@

# # nlohmann's json does not need to be compiled individually


#for debug of station
station.exe: station.o pugixml.o
	${CXX} ${CXXFLAGS} $^ ${LDFLAGS} -o $@

clean-db:
	-rm nyc-subway-tracker.db

#==============================================================================

ROUTES_TARGET := routes_fix.exe

# for routes_fix for processing routes.json
routes_fix: ${ROUTES_TARGET}

${ROUTES_TARGET}: routes_fix.o
	${CXX} ${CXXFLAGS} $< -o src/misc/$@

routes_fix.o: src/misc/routes_fix.cpp
	${CXX} ${CXXFLAGS} -c $< -o $@