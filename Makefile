.PHONY: all _all release _target build-external openssl curl pugixml sqlite remake directories clean cleaner cleandb
.DEFAULT_GOAL: all
# .SILENT:

# ===== MAKE AND CXX ENV =======================================================

# Make Flags
MYMAKEFLAGS		:= --no-print-directory

# File Extensions
SRCEXT			:= cpp
INCEXT			:= h
PCHEXT			:= gch
OBJEXT			:= o
TARGETEXT		:= 
SHELLEXT		:= sh

# Paths
SRCPATH 		:= src
INCPATH			:= include
EXTPATH			:= lib
BUILDPATH 		:= build
BINPATH 		:= bin

BUILDLOG 		:= $(BUILDPATH)/build.log

# Compiler
CXX 			:= g++
CXXFLAGS 		:= -g -std=c++17 -O3 -Wall -Wextra

ifdef RELEASE
    CXXFLAGS += -static -static-libgcc -static-libstdc++
endif

PCHSRC			:= $(INCPATH)/pch.$(INCEXT)
PCHDST			:= $(BUILDPATH)/pch.$(INCEXT)
PCHBUILD		:= $(PCHDST).$(PCHEXT)

# ===== EXTERNAL BUILD =========================================================

# External dependencies
OPENSSL_LIBPATH := ${BUILDPATH}/openssl
CURL_LIBPATH	:= ${BUILDPATH}/curl/lib/.libs/
SQLITE_LIBPATH	:= ${BUILDPATH}/sqlite/.libs/

OPENSSLTARGET	:= $(OPENSSL_LIBPATH)/libssl.a
CURLTARGET		:= $(CURL_LIBPATH)libcurl.a
PUGIXMLTARGET	:= $(BUILDPATH)/pugixml.$(OBJEXT)
SQLITETARGET	:= $(SQLITE_LIBPATH)/libsqlite3.a

DEPOBJECTS		:= \
				   $(PUGIXMLTARGET)

# Includes and Links
INCFLAGS		:= \
				   -I $(BUILDPATH) \
				   -I $(INCPATH) \
				   -I ${EXTPATH}/openssl/include \
				   -I ${EXTPATH}/curl/include \
				   -I ${EXTPATH}/nlohmann/single_include \
				   -I ${EXTPATH}/pugixml/src \
				   -I ${BUILDPATH}/sqlite

LDFLAGS 		:= \
				   -L${BUILDPATH}/openssl -lssl -lcrypto \
				   -L${BUILDPATH}/curl/lib/.libs -lcurl \
                   -L${BUILDPATH}/sqlite/.libs -lsqlite3 \

OPENSSL_INSTALLPATH	:= $(shell pwd)/$(BUILDPATH)/openssl/install
OPENSSLFLAG		:= no-tests --prefix=$(OPENSSL_INSTALLPATH) --openssldir=$(OPENSSL_INSTALLPATH)
CURLFLAG		:= --with-openssl=$(OPENSSL_INSTALLPATH) --without-libpsl

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
ifneq ($(filter Linux, $(shell uname)),) 		# Linux
    OS				:= linux
    TARGETEXT 		:= 
else ifneq ($(filter Darwin, $(shell uname)),) 	# macOS
    OS				:= darwin
    TARGETEXT		:=
else ifneq ($(findstring NT, $(shell uname)),) 	# Windows (msys2 or cygwin)
    OS				:= windows
    TARGETEXT 		:= .exe
else
    OS				:= 
endif

# Target
TARGETNAME		:= subway-logger
TARGET 			:= ${BINPATH}/$(TARGETNAME)${TARGETEXT}

# ===== BUILD MY OBJECTS ======================================================

# wrapper to pass our makeflags into the make call
all:
#	@echo [debug] the OS is $(OS)
	@if [ "$(OS)" = "" ]; then exit 1; fi

	@$(MAKE) -$(MAKEFLAGS) \
	$(MYMAKEFLAGS) \
	$(if $(findstring j, $(MAKEFLAGS)),,-j) \
	_all
_all: directories $(EXTBUILDS) _target

# tells makefile to add static linking to CXXFLAGS when run
release:
	@$(MAKE) -$(MAKEFLAGS) $(MYMAKEFLAGS) --eval="RELEASE=0" all

# wrapper to pass our makeflags into our make call to make our objects
_target: directories $(EXTBUILDS)
	@$(MAKE) -$(MAKEFLAGS) $(MYMAKEFLAGS) $(TARGET)

${TARGET}: ${OBJECTS} $(DEPOBJECTS)
	@echo building $@...; \
	echo ${CXX} ${CXXFLAGS} ${INCFLAGS} $^ $(LDFLAGS) -o $@; \
	echo

	@${CXX} ${CXXFLAGS} ${INCFLAGS} $^ $(LDFLAGS) -o $@
	@if [ "$(OS)" = "darwin" ]; then \
	echo DYLD_LIBRARY_PATH=\
	$(OPENSSL_LIBPATH):$(CURL_LIBPATH):$(SQLITE_LIBPATH) bin/subway-logger \
	> $(BINPATH)/$(TARGETNAME).$(SHELLEXT) $$\@; \
	fi

	@echo \[$(shell date +%Y-%m-%d-%H:%M:%S)\] \[$(shell uname -srm)\] built $@ >> $(BUILDLOG)

${OBJECTS}: ${BUILDPATH}/%.${OBJEXT}: ${SRCPATH}/%.${SRCEXT}
	@echo building $@...; \
	echo ${CXX} -c ${CXXFLAGS} ${INCFLAGS} $< -o $@; \
	echo

	@${CXX} -c ${CXXFLAGS} ${INCFLAGS} $< -o $@

	@echo \[$(shell date +%Y-%m-%d-%H:%M:%S)\] \[$(shell uname -srm)\] built $@ >> $(BUILDLOG)

# sadly it seems that g++/gcc doesn't benefit much from pch and having one
# uses an additional ~50 MB for build, so it's better not to use one
# $(PCHBUILD): $(PCHSRC)
# 	@echo building $@...; \
# 	echo $(CXX) $(CXXFLAGS) -x c++-header $(INCFLAGS) $< -o $(PCHBUILD); \
# 	echo
# 	@cp $(PCHSRC) $(PCHDST)
# 	@$(CXX) $(CXXFLAGS) -x c++-header $(INCFLAGS) $< -o $(PCHBUILD)
# 	@echo \[$(shell date +%Y-%m-%d-%H:%M:%S)\] \[$(shell uname -srm)\] built $@ >> $(BUILDLOG)


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
	../../${EXTPATH}/openssl/Configure $(OPENSSLFLAG); \
	$(MAKE) $(MYMAKEFLAGS); \
	$(MAKE) $(MYMAKEFLAGS) install_sw

	@echo \[$(shell date +%Y-%m-%d-%H:%M:%S)\] \[$(shell uname -srm)\] built $@ >> $(BUILDLOG)

$(CURLTARGET): $(OPENSSLTARGET)
	@echo building curl...
	@cd ${EXTPATH}/curl; \
	autoreconf -fi; \
	mkdir ../../${BUILDPATH}/curl; cd ../../${BUILDPATH}/curl; \
	../../${EXTPATH}/curl/configure $(CURLFLAG)||:; \
	$(MAKE) $(MYMAKEFLAGS)

	@echo \[$(shell date +%Y-%m-%d-%H:%M:%S)\] \[$(shell uname -srm)\] built $@ >> $(BUILDLOG)

$(PUGIXMLTARGET): ${PUGIXMLSRC}
	@echo building $@...; \
	echo ${CXX} -c ${CXXFLAGS} ${INCFLAGS} $< -o ${PUGIXMLTARGET}

	@${CXX} -c ${CXXFLAGS} ${INCFLAGS} $< -o ${PUGIXMLTARGET}

	@echo \[$(shell date +%Y-%m-%d-%H:%M:%S)\] \[$(shell uname -srm)\] built $@ >> $(BUILDLOG)

# I don't know why, but for some reason the linker fails on the first make
# attempt, but then succeeds on the second one???? So the submake is called thrice
# to be very sure that compilation succeeds
$(SQLITETARGET):
	@echo building sqlite...
	@cd $(EXTPATH)/sqlite; \
	autoreconf -fi; \
	mkdir ../../${BUILDPATH}/sqlite; cd ../../${BUILDPATH}/sqlite; \
	../../${EXTPATH}/sqlite/configure; \
	$(MAKE) $(MYMAKEFLAGS) || $(MAKE) $(MYMAKEFLAGS) || $(MAKE) $(MYMAKEFLAGS)
	@echo \[$(shell date +%Y-%m-%d-%H:%M:%S)\] \[$(shell uname -srm)\] built $@ >> $(BUILDLOG)

# ===== MISC ==================================================================

directories:
	-@mkdir -p ${BUILDPATH}
	-@mkdir -p ${BINPATH}

remake: clean all

clean: 
	-rm -rf $(BINPATH)
	-rm -rf $(OBJECTS)
#	-rm -rf $(PCHDST) $(PCHBUILD)
	
cleaner: clean
	-rm -rf ${BUILDPATH}

cleandb:
	-rm -f nyc-subway-tracker.db
