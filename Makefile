.PHONY: all _target build-external openssl curl pugixml sqlite remake directories clean cleaner cleandb
.DEFAULT_GOAL: all
# .SILENT:

# Make Flags
MYMAKEFLAGS		:= -j --no-print-directory

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
CXXFLAGS 		:= -std=c++17 -O3 -Wall -Wextra #-g

# Includes and Links
INCFLAGS		+= -I include \
				   -I ${EXTPATH}/openssl/include \
				   -I ${EXTPATH}/curl/include \
				   -I ${EXTPATH}/nlohmann/single_include \
				   -I ${EXTPATH}/pugixml/src \
				   -I ${BUILDPATH}/sqlite

LDFLAGS 		:= -L ${BUILDPATH}/openssl -l ssl \
				   -L ${BUILDPATH}/curl/lib/.libs -l curl \
                   -L ${BUILDPATH}/sqlite/.libs -l sqlite3 

# Build and Link Externals
EXTBUILDS 		:= 

ifeq ($(wildcard ${BUILDPATH}/openssl/libssl.a),)
    EXTBUILDS += openssl
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

CURLFLAG		:= --without-ssl
OPENSSLFLAG		:= no-tests no-unit-test

PUGIXMLSRC		:= ${EXTPATH}/pugixml/src/pugixml.cpp
PUGIXMLOBJ		:= ${BUILDPATH}/pugixml.${OBJEXT}

DEPOBJECTS		:= ${PUGIXMLOBJ}

# Build (Project Sources and Objects)
SOURCES 		:= $(wildcard $(SRCPATH)/*.${SRCEXT})
OBJECTS 		:= $(patsubst ${SRCPATH}/%.${SRCEXT},${BUILDPATH}/%.${OBJEXT},${SOURCES})

# Adjust Variables depending on Environment
ifeq ($(filter ${shell uname}, linux),) #Linux
    TARGETEXT 	:= 
else ifeq ($(filter ${shell uname}, darwin),) #macOS
    TARGETEXT 	:= 
else ifeq ($(filter $(shell ver), windows),) #Windows
	TARGETEXT 	:= .exe
else
	exit 1
endif

# Target
TARGET 			:= ${TARGETPATH}/subway-logger${TARGETEXT}

# ===== BUILD MY OBJECTS ======================================================

all: directories build-external _target

# for releases, compiles statically
release: _release all
_release:
	@$(eval CXXFLAGS += -static -static-libgcc -static-libstdc++)

_target:
	@make $(MYMAKEFLAGS) $(TARGET)

${TARGET}: ${OBJECTS} ${DEPOBJECTS} 
	@echo building $@...; \
	echo ${CXX} ${CXXFLAGS} ${INCFLAGS} $^ $(LDFLAGS) -o $@; \
	echo
	@${CXX} ${CXXFLAGS} ${INCFLAGS} $^ $(LDFLAGS) -o $@

${OBJECTS}: ${BUILDPATH}/%.${OBJEXT}: ${SRCPATH}/%.${SRCEXT}
	@echo building $@...; \
	echo ${CXX} -c ${CXXFLAGS} ${INCFLAGS} $< $(LDFLAGS) -o $@; \
	echo
	@${CXX} -c ${CXXFLAGS} ${INCFLAGS} $< $(LDFLAGS) -o $@

# nlohmann's json does not need to be compiled individually

# ===== BUILD EXT OBJECTS =====================================================

build-external: ${EXTBUILDS}
	
# for most of these, the process is:
# (sometimes)
# 1. cd into the external library's directory 
# 2. run autoreconf -fi
# (always)
# 3. make the build directory and then cd into it
# 4. run configure when in the build directory
# 5. run make

openssl:
	@echo building openssl for curl...
	@mkdir ${BUILDPATH}/openssl; cd ${BUILDPATH}/openssl; \
	../../${EXTPATH}/openssl/configure $(OPENSSLFLAG); \
	make $(MYMAKEFLAGS); \
	echo; \
	echo ========================================

curl:
	@echo building curl...
	@cd ${EXTPATH}/curl; \
	autoreconf -fi; \
	mkdir ../../${BUILDPATH}/curl; cd ../../${BUILDPATH}/curl; \
	../../${EXTPATH}/curl/configure $(CURLFLAG)||:; \
	make $(MYMAKEFLAGS); \
	echo; \
	echo ========================================

pugixml: ${PUGIXMLSRC}
	@echo building $@...; \
	echo ${CXX} -c ${CXXFLAGS} ${INCFLAGS} $< -o ${PUGIXMLOBJ}; \
	echo; \
	echo ========================================
	@${CXX} -c ${CXXFLAGS} ${INCFLAGS} $< -o ${PUGIXMLOBJ}; \

sqlite:
	@echo building sqlite...
	@cd $(EXTPATH)/sqlite; \
	autoreconf -fi; \
	mkdir ../../${BUILDPATH}/sqlite; cd ../../${BUILDPATH}/sqlite; \
	../../${EXTPATH}/sqlite/configure; \
	make $(MYMAKEFLAGS); \
	make $(MYMAKEFLAGS) sqlite3.c; \
	echo; \
	echo ========================================

# ===== MISC ==================================================================

directories:
	-@mkdir -p ${BUILDPATH}
	-@mkdir -p ${TARGETPATH}

remake: cleaner
	make

clean: 
	-rm -rf $(TARGETPATH)
	-rm -rf $(OBJECTS)
	
cleaner: clean
	-rm -rf ${BUILDPATH}

cleandb:
	-rm -f nyc-subway-tracker.db
