.PHONY: all _all release _target build-external openssl curl pugixml sqlite remake directories clean cleaner cleandb
.DEFAULT_GOAL: all
# .SILENT:

# ===== MAKE AND CXX ENV =======================================================

# Make Flags
MYMAKEFLAGS		:= --no-print-directory

# File Extensions
SRCEXT			:= cpp
OBJEXT			:= o
TARGETEXT		:= 

# Paths
SRCPATH 		:= src
EXTPATH			:= external
BUILDPATH 		:= build
TARGETPATH 		:= bin

BUILDLOG 		:= $(BUILDPATH)/build.log

# Compiler
CXX 			:= g++
CXXFLAGS 		:= -g -std=c++17 -O0 -Wall -Wextra

ifdef RELEASE
    CXXFLAGS += -static -static-libgcc -static-libstdc++
endif

# ===== EXTERNAL BUILD =========================================================

# External dependencies
OPENSSLTARGET	:= ${BUILDPATH}/openssl/libssl.a
CURLTARGET		:= ${BUILDPATH}/curl/lib/.libs/libcurl.a
PUGIXMLTARGET	:= ${BUILDPATH}/pugixml.${OBJEXT}
SQLITETARGET	:= ${BUILDPATH}/sqlite/.libs/libsqlite3.a

DEPOBJECTS		:= \
				   $(PUGIXMLTARGET)

# Includes and Links
INCFLAGS		+= \
				   -I include \
				   -I ${EXTPATH}/openssl/include \
				   -I ${EXTPATH}/curl/include \
				   -I ${EXTPATH}/nlohmann/single_include \
				   -I ${EXTPATH}/pugixml/src \
				   -I ${BUILDPATH}/sqlite

LDFLAGS 		:= \
				   -L ${BUILDPATH}/openssl -l ssl -l crypto \
				   -L ${BUILDPATH}/curl/lib/.libs -l curl \
                   -L ${BUILDPATH}/sqlite/.libs -l sqlite3 \

OPENSSLFLAG		:= no-tests
CURLFLAG		:= --with-openssl=$(shell pwd)/$(BUILDPATH)/openssl

PUGIXMLSRC		:= ${EXTPATH}/pugixml/src/pugixml.cpp

# External builds that our target needs
EXTBUILDS 		:= \
				   $(OPENSSLTARGET) \
				   $(CURLTARGET)	\
				   $(PUGIXMLTARGET) \
				   $(SQLITETARGET) 

# ===== PROJECT BUILD ==========================================================

# Build (Project Sources and Objects)
SOURCES 		:= $(wildcard $(SRCPATH)/*.${SRCEXT})
OBJECTS 		:= $(patsubst ${SRCPATH}/%.${SRCEXT},${BUILDPATH}/%.${OBJEXT},${SOURCES})

# Adjust variables depending on environment
ifeq ($(filter ${shell uname}, linux),) 		# Linux
    TARGETEXT 	:= 
else ifeq ($(filter ${shell uname}, darwin),) 	# macOS
    TARGETEXT 	:= 
else ifeq ($(findstring $(shell uname), NT),) 	# Windows (msys2 or cygwin)
    TARGETEXT 	:= .exe
else
    exit 1
endif

# Target
TARGET 			:= ${TARGETPATH}/subway-logger${TARGETEXT}

# ===== BUILD MY OBJECTS ======================================================

# wrapper to pass our makeflags into the make call
all:
	@$(MAKE) -$(MAKEFLAGS) \
	$(MYMAKEFLAGS) \
	$(if $(findstring j, $(MAKEFLAGS)),,-j) \
	_all
_all: directories ${EXTBUILDS} _target

# tells makefile to add static linking to CXXFLAGS when run
release:
	@$(MAKE) -$(MAKEFLAGS) $(MYMAKEFLAGS) --eval="RELEASE=0" all

# wrapper to pass our makeflags into our make call to make our objects
_target: directories ${EXTBUILDS}
	@$(MAKE) -$(MAKEFLAGS) $(MYMAKEFLAGS) $(TARGET)

${TARGET}: ${OBJECTS} $(DEPOBJECTS)
	@echo building $@...; \
	echo ${CXX} ${CXXFLAGS} ${INCFLAGS} $^ $(LDFLAGS) -o $@; \
	echo
	@${CXX} ${CXXFLAGS} ${INCFLAGS} $^ $(LDFLAGS) -o $@
	@echo \[$(shell date +%Y-%m-%d-%H:%M:%S)\] \[$(shell uname -srm)\] built $@ >> $(BUILDLOG)

${OBJECTS}: ${BUILDPATH}/%.${OBJEXT}: ${SRCPATH}/%.${SRCEXT}
	@echo building $@...; \
	echo ${CXX} -c ${CXXFLAGS} ${INCFLAGS} $< $(LDFLAGS) -o $@; \
	echo
	@${CXX} -c ${CXXFLAGS} ${INCFLAGS} $< $(LDFLAGS) -o $@
	@echo \[$(shell date +%Y-%m-%d-%H:%M:%S)\] \[$(shell uname -srm)\] built $@ >> $(BUILDLOG)

# nlohmann's json does not need to be compiled individually

# ===== BUILD EXT OBJECTS =====================================================
	
# for most of these, the process is:
# (sometimes)
# 1. cd into the external library's directory 
# 2. run autoreconf -fi
# (always)
# 3. make the build directory and then cd into it
# 4. run configure when in the build directory
# 5. run make

$(OPENSSLTARGET):
	@echo building openssl for curl...
	@mkdir ${BUILDPATH}/openssl; cd ${BUILDPATH}/openssl; \
	../../${EXTPATH}/openssl/configure $(OPENSSLFLAG); \
	make $(MYMAKEFLAGS); \
	echo; \
	echo ========================================
	@echo \[$(shell date +%Y-%m-%d-%H:%M:%S)\] \[$(shell uname -srm)\] built $@ >> $(BUILDLOG)

$(CURLTARGET): $(OPENSSLTARGET)
	@echo building curl...
	@cd ${EXTPATH}/curl; \
	autoreconf -fi; \
	mkdir ../../${BUILDPATH}/curl; cd ../../${BUILDPATH}/curl; \
	../../${EXTPATH}/curl/configure $(CURLFLAG)||:; \
	make $(MYMAKEFLAGS); \
	echo; \
	echo ========================================
	@echo \[$(shell date +%Y-%m-%d-%H:%M:%S)\] \[$(shell uname -srm)\] built $@ >> $(BUILDLOG)

$(PUGIXMLTARGET): ${PUGIXMLSRC}
	@echo building $@...; \
	echo ${CXX} -c ${CXXFLAGS} ${INCFLAGS} $< -o ${PUGIXMLTARGET}; \
	echo; \
	echo ========================================
	@${CXX} -c ${CXXFLAGS} ${INCFLAGS} $< -o ${PUGIXMLTARGET}
	@echo \[$(shell date +%Y-%m-%d-%H:%M:%S)\] \[$(shell uname -srm)\] built $@ >> $(BUILDLOG)

$(SQLITETARGET):
	@echo building sqlite...
	@cd $(EXTPATH)/sqlite; \
	autoreconf -fi; \
	mkdir ../../${BUILDPATH}/sqlite; cd ../../${BUILDPATH}/sqlite; \
	../../${EXTPATH}/sqlite/configure; \
	make $(MYMAKEFLAGS); \
	make $(MYMAKEFLAGS) sqlite3.c; \
	echo; \
	echo ========================================
	@echo \[$(shell date +%Y-%m-%d-%H:%M:%S)\] \[$(shell uname -srm)\] built $@ >> $(BUILDLOG)

# ===== MISC ==================================================================

directories:
	-@mkdir -p ${BUILDPATH}
	-@mkdir -p ${TARGETPATH}

remake: clean all

clean: 
	-rm -rf $(TARGETPATH)
	-rm -rf $(OBJECTS)
	
cleaner: clean
	-rm -rf ${BUILDPATH}

cleandb:
	-rm -f nyc-subway-tracker.db
