.PHONY: all build-external curl pugixml sqlite remake directories clean cleaner cleandb
.DEFAULT_GOAL: all
# .SILENT:

# File Extensions
SRCEXT			:= cpp
OBJEXT			:= o
TARGETEXT		:= 

# Paths
SRCPATH 		:= src
EXTPATH			:= external
BUILDPATH 		:= build
TARGETPATH 		:= bin

# Compiler
CXX 			:= g++
CXXFLAGS 		:= -std=c++17 -g -O2 -Wall
CXXTFLAGS		:= -static
INCFLAGS		:= -I include

# Includes
INCFLAGS		+= -I ${EXTPATH}/curl/include \
				   -I ${EXTPATH}/nlohmann/single_include \
				   -I ${EXTPATH}/pugixml/src \
				   -I ${BUILDPATH}/sqlite

# Build and Link Externals
EXTBUILDS 		:= 
CURLFLAG		:= 

PUGIXMLSRC		:= ${EXTPATH}/pugixml/src/pugixml.cpp
PUGIXMLOBJ		:= ${BUILDPATH}/pugixml.${OBJEXT}

DEPOBJECTS		:= ${PUGIXMLOBJ}

LDFLAGS 		:= -L ${BUILDPATH}/curl/lib/.libs -l curl 
LDFLAGS 		+= -L ${BUILDPATH}/sqlite/.libs -l sqlite3 

# Build (Project Sources and Objects)
SOURCES 		:= $(wildcard $(SRCPATH)/*.${SRCEXT})
OBJECTS 		:= $(patsubst ${SRCPATH}/%.${SRCEXT},${BUILDPATH}/%.${OBJEXT},${SOURCES})

# Adjust Variables depending on Environment
ifeq ($(filter ${shell uname}, linux),) #Linux
    TARGETEXT 	:= 
    CURLFLAG	+= --with-openssl
else ifeq ($(filter ${shell uname}, darwin),) #macOS
    TARGETEXT 	:= 
    CURLFLAG	+= --with-openssl=/opt/homebrew/opt/openssl
else ifeq ($(filter $(shell ver), windows),) #Windows
	TARGETEXT 	:= .exe
    CURLFLAG	+= --with-openssl
else
	exit 1
endif

# Find Which Libraries Need to Be Built
ifeq ($(wildcard ${BUILDPATH}/curl/lib/.libs/libcurl.a),)
    EXTBUILDS += curl
endif

ifeq ($(wildcard ${BUILDPATH}/pugixml.${OBJEXT}),)
    EXTBUILDS += pugixml
endif

ifeq ($(wildcard ${BUILDPATH}/sqlite/sqlite3.h),)
    EXTBUILDS += sqlite
endif

# Target
TARGET 			:= ${TARGETPATH}/subway-logger${TARGETEXT}

# ===== BUILD MY OBJECTS ======================================================

all: directories build-external ${TARGET}

${TARGET}: ${OBJECTS} ${DEPOBJECTS} 
	@echo building $@...
	${CXX} ${CXXFLAGS} ${INCFLAGS} $^ $(LDFLAGS) -o $@ ||:
	@echo ========================================
	@echo

${OBJECTS}: ${BUILDPATH}/%.${OBJEXT}: ${SRCPATH}/%.${SRCEXT}
	@echo building $@...
	${CXX} -c ${CXXFLAGS} ${INCFLAGS} $< $(LDFLAGS) -o $@ ||:
	@echo ========================================
	@echo

# nlohmann's json does not need to be compiled individually

# ===== BUILD EXT OBJECTS =====================================================

build-external: directories ${EXTBUILDS}
	
curl:
	@echo building curl...
	@echo
	@cd ${EXTPATH}/curl ||:; \
	autoreconf -fi ||:; \
	cd ../.. ||:; \
	mkdir ${BUILDPATH}/curl ||:; \
	cd ${BUILDPATH}/curl ||:; \
	../../${EXTPATH}/curl/configure $(CURLFLAG)||:; \
	make ||:; 

pugixml: ${PUGIXMLSRC}
	@echo building pugixml...
	@echo
	@${CXX} -c ${CXXFLAGS} ${INCFLAGS} $< -o ${PUGIXMLOBJ} ||:

sqlite:
	@echo building sqlite...
	@echo
	@mkdir ${BUILDPATH}/sqlite ||:; \
	cd ${BUILDPATH}/sqlite ||:; \
	../../${EXTPATH}/sqlite/configure ||:; \
	make ||:; \
	make sqlite3.c ||:;

# ===== MISC ==================================================================

directories:
	-@mkdir -p ${BUILDPATH} ||:
	-@mkdir -p ${TARGETPATH} ||:

remake: cleaner
	make

clean: 
	-rm -rf $(TARGETPATH)
	-rm -rf $(OBJECTS)
	
cleaner: clean
	-rm -rf ${BUILDPATH}

cleandb:
	-rm -f nyc-subway-tracker.db
