.PHONY: all build-external curl pugixml sqlite remake directories clean cleaner cleandb

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
CXXFLAGS 		:= -std=c++17 -g -O2 #-Wall
INCFLAGS		:= -I include

# Includes
INCFLAGS		+= -I ${EXTPATH}/curl/include \
				   -I ${EXTPATH}/nlohmann/single_include \
				   -I ${EXTPATH}/pugixml/src \
				   -I ${BUILDPATH}/sqlite			

# Build and Link Externals
EXTBUILDS 		:= 
OPENSSLFLAG		:= 

PUGIXMLSRC		:= ${EXTPATH}/pugixml/src/pugixml.cpp
PUGIXMLOBJ		:= ${BUILDPATH}/pugixml.${OBJEXT}

DEPOBJECTS		:= ${PUGIXMLOBJ}

LDFLAGS 		:= -L ${BUILDPATH}/curl/lib/.libs -l curl 
LDFLAGS 		+= -L ${BUILDPATH}/sqlite/.libs -l sqlite3 

# Build (Project Sources and Objects)
SOURCES 		:= $(wildcard $(SRCPATH)/*.${SRCEXT})
OBJECTS 		:= $(patsubst ${SRCPATH}/%.${SRCEXT},${BUILDPATH}/%.${OBJEXT},${SOURCES})

# Adjust Variables depending on Environment
ifeq ($(filter ${shell uname}, Linux), Linux) #Linux
    TARGETEXT 	:= 
    OPENSSLFLAG	:= --with-openssl
else ($(filter ${shell uname}, Darwin), Darwin) #macOS
    TARGETEXT 	:= 
    OPENSSLFLAG	:= --with-openssl=/opt/homebrew/opt/openssl
else #Windows
	TARGETEXT 	:= .exe
    OPENSSLFLAG	:= --with-openssl
endif

# Target
TARGET 			:= ${TARGETPATH}/subway-logger${TARGETEXT}

# =============================================================================
all: directories build-external ${TARGET}

${TARGET}: ${OBJECTS} ${DEPOBJECTS} 
	@echo building ${TARGET}...
	@echo
	@${CXX} ${CXXFLAGS} ${INCFLAGS} $^ ${LDFLAGS} -o $@ ||:

${OBJECTS}: ${BUILDPATH}/%.${OBJEXT}: ${SRCPATH}/%.${SRCEXT}
	@echo building $<...
	@echo
	@${CXX} -c ${CXXFLAGS} ${INCFLAGS} $< -o $@ ||:

# nlohmann's json does not need to be compiled individually

# =============================================================================

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

build-external: directories ${EXTBUILDS}
	
curl:
	@echo building curl...
	@echo
	@cd ${EXTPATH}/curl ||:; \
	autoreconf -fi ||:; \
	cd ../.. ||:; \
	mkdir ${BUILDPATH}/curl ||:; \
	cd ${BUILDPATH}/curl ||:; \
	../../${EXTPATH}/curl/configure ${OPENSSLFLAG} ||:; \
	make ||:; 

pugixml: ${PUGIXMLSRC}
	@echo building pugixml...
	@echo
	@${CXX} -c ${CXXFLAGS} ${INCFLAGS} $< -o ${PUGIXMLOBJ} ||:

sqlite:
	@echo building sqlite...
	@echo step1
	@mkdir ${BUILDPATH}/sqlite ||:; \
	echo step2 ||:; \
	cd ${BUILDPATH}/sqlite ||:; \
	echo step3 ||:; \
	../../${EXTPATH}/sqlite/configure ||:; \
	echo step4 ||:; \
	make ||:; \
	echo step5 ||:; \
	make sqlite3.c ||:;

# =============================================================================

remake: cleaner all

directories:
	@mkdir ${BUILDPATH} ||:
	@mkdir ${TARGETPATH} ||:

clean: 
	-rm -rf ${BUILDPATH}
	@echo cleaning curl...
	@cd ${EXTPATH}/curl ||:; \
	git clean -fdx ||:; \

cleaner: clean
	-rm -rf ${TARGETPATH}

cleandb:
	-rm -f nyc-subway-tracker.db

# =============================================================================
# Unused
# =============================================================================

ROUTES_TARGET := routes_fix.exe

routes_fix: ${ROUTES_TARGET}

${ROUTES_TARGET}: routes_fix.o
	${CXX} ${CXXFLAGS} $< -o src/misc/$@

routes_fix.o: src/misc/routes_fix.cpp
	${CXX} ${CXXFLAGS} -c $< -o $@