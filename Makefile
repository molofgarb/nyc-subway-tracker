.PHONY: all remake directories clean cleaner cleandb

# File Extensions
SRCEXT		:= cpp
OBJEXT		:= o
TARGETEXT	:= exe

# Adjust TARGETEXT based on OS
ifneq (,$(filter ${shell uname}, Linux Darwin)) #Linux or macOS, WIP
TARGETEXT 	:= 
endif

# Paths
SRCPATH 	:= src
LIBSPATH	:= libs
BUILDPATH 	:= build
TARGETPATH 	:= bin

# Compiler
CXX 		:= g++
CXXFLAGS 	:= -std=c++17 -g -O3 #-Wall
INCFLAGS	:= -I include

# Includes
INCFLAGS	+= -I ${LIBSPATH}/curl/include
INCFLAGS	+= -I ${LIBSPATH}/nlohmann/single_include
INCFLAGS	+= -I ${LIBSPATH}/pugixml/src
INCFLAGS	+= -I ${LIBSPATH}/sqlite/build			

# Build and Link Externals
PUGIXMLSRC	:= ${LIBSPATH}/pugixml/src/pugixml.cpp
PUGIXMLOBJ	:= ${BUILDPATH}/pugixml.${OBJEXT}

DEPOBJECTS	:= ${PUGIXMLOBJ}

LDFLAGS 	:= -L ${LIBSPATH}/curl/lib/.libs -l curl 
LDFLAGS 	+= -L ${LIBSPATH}/sqlite/build/.libs -l sqlite3 

# Build (Project Sources and Objects)
SOURCES 	:= $(wildcard $(SRCPATH)/*.${SRCEXT})
OBJECTS 	:= $(patsubst ${SRCPATH}/%.${SRCEXT},${BUILDPATH}/%.${OBJEXT},${SOURCES})

# Target
TARGET 		:= ${TARGETPATH}/nyc-subway-tracker.${TARGETEXT}

# =============================================================================

all: directories ${TARGET}

${TARGET}: ${OBJECTS} ${DEPOBJECTS}
	@echo 
	${CXX} ${CXXFLAGS} ${INCFLAGS} $^ ${LDFLAGS} -o $@

${OBJECTS}: ${BUILDPATH}/%.${OBJEXT}: ${SRCPATH}/%.${SRCEXT}
	@echo
	${CXX} -c ${CXXFLAGS} ${INCFLAGS} $< -o $@

${PUGIXMLOBJ}: ${PUGIXMLSRC}
	@echo
	${CXX} -c ${CXXFLAGS} ${INCFLAGS} $< -o $@

# nlohmann's json does not need to be compiled individually

# =============================================================================

remake: cleaner all

directories:
	-mkdir ${BUILDPATH}
	-mkdir ${TARGETPATH}

clean: 
	-rm -f ${BUILDPATH}

cleaner: 
	-rm -rf ${BUILDPATH}
	-rm -rf ${TARGETPATH}

cleandb:
	-rm -f ${BUILDPATH}/nyc-subway-tracker.db

# =============================================================================
# Unused
# =============================================================================

ROUTES_TARGET := routes_fix.exe

routes_fix: ${ROUTES_TARGET}

${ROUTES_TARGET}: routes_fix.o
	${CXX} ${CXXFLAGS} $< -o src/misc/$@

routes_fix.o: src/misc/routes_fix.cpp
	${CXX} ${CXXFLAGS} -c $< -o $@