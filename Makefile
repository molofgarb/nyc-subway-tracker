.PHONY: all clean

CC := g++
FLAGS := -std=c++17 -O2
LINKS := -L .\src\curl\lib\.libs -l libcurl
# links: libcurl library

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
${TARGET}: main.o station.o line.o getPage.o pugixml.o
	${CC} ${FLAGS} $^ ${LINKS} -o $@

main.o: src/main.cpp 
	${CC} -c ${FLAGS} $^ -o $@

station.o: src/station.cpp 
	${CC} -c ${FLAGS} $^ -o $@

line.o: src/line.cpp
	${CC} -c ${FLAGS} $^ -o $@

getPage.o: src/getPage.cpp
	${CC} -c ${FLAGS} $^ -o $@


pugixml.o: src/pugixml/src/pugixml.cpp # compile pugixml
	${CC} -c ${FLAGS} $^ -o $@

# nlohmann's json does not need to be compiled individually


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