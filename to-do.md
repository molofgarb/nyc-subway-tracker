# To-do:

- add git submodule cloning stuff to make to simplify build process
- **need to install tcl, which is a dependency of sqlite**
- add/modify custom struct to track end boroughs for a line (stop headsign in xml), to specify direction

## **High Priority**
- subway-logger.exe and subway-viewer.exe as separate programs to interact with subway.db
- exceptions for table operations in tracker - what if row/table doesn't exist?
- make sure there is a valid output by operator<< if station/line/subway is not updated after initializing
- support use of line and station by itself (need to rework destructor)
  - support empty stationMap/stationtypes if line used in main script instead of subway

## **Medium Priority**
- delete oldest records if db file is too big
- make subway output nicer (multiple files in a directory option)
- exception handling (in general)

## **Low Priority**:
- use multithreading in line/subway update
- if libcurl get error, revisit station to update again later
- progress bar?


## Completed
- add line tables and line-based subway table
- optimize space of sql table by using appropriate types and enforcing quotes on all data
- compile submodules as part of make
- add include flags thing to make to make includes nicer
- getPage into string to reduce overhead
- log time in db for arrival as normal format instead of unix
- fix time until arrival bug in tracker and station
- sqlite3 stuff
- build curl and sqlite as part of makefile process
- Download DB Browser for SQLite to view subway.db 
- pointerize tracker.cpp
- fix const structs with useless functions
- make sure that environment is detected correctly in makefile
- fix const structs with useless functions
- fix multiple options for subway output
- have station output say when it was checked in output operator
- format time more nicely
- use **std::shared_ptr** for Station* since shared by Line and Subway and fix Line destructor
- use **custom struct** instead of std::pair in station
- what happens if one of the temp files is deleted while in progress?

# Notes
- Only keep last 30 days of data or a certain limit -- if data is older than current date and
  the current amount of data is greater than the limit, then delete to meet the limit starting from oldest
- Make sure to scrape at not too quick of an interval that bad things happen, but enough so that every train is captured
- Should be able to create a table to uniquely identify a train as it progresses through stations

## Table Structurein tracker.cpp
- Subway keeps stations
  - Station keeps nearby
- Subway keeps lines
  - Lines keeps stations
    - Stations keeps nearby

## Json convert:
  1. { --> \n{\n    
  2. } --> \n}
  3. , --> ,\n    
  4. },\n --> },
  5. ] --> \n]

## URLs
main: 
qeqy84JE7hUKfaI0Lxm2Ttcm6ZA0bYrP - **alerts** (for ALL forms of transport - dynamic) (IGNORE)
- this could be used to track frequency of delays on each train -- may be implemented in future


Z276E3rCeTzOQEoBPPN4JCEc6GfvdnYE - **routes** (for ALL forms of transport - static) (kept in resources)
  https://otp-mta-prod.camsys-apps.com/otp/routers/default/index/routes?apikey=Z276E3rCeTzOQEoBPPN4JCEc6GfvdnYE


qeqy84JE7hUKfaI0Lxm2Ttcm6ZA0bYrP - **stopsForRoute** (after clicking on a train - static)
  https://collector-otp-prod.camsys-apps.com/schedule/MTASBWY/stopsForRoute?apikey=qeqy84JE7hUKfaI0Lxm2Ttcm6ZA0bYrP&&routeId=MTASBWY:<LINE DESIGNATION>


Z276E3rCeTzOQEoBPPN4JCEc6GfvdnYE - **nearby** (after clicking on a stop - dynamic)
  https://otp-mta-prod.camsys-apps.com/otp/routers/default/nearby?stops=MTASBWY%3AG14&apikey=Z276E3rCeTzOQEoBPPN4JCEc6GfvdnYE
  - above is example nearby for roosevelt av in queens
  https://otp-mta-prod.camsys-apps.com/otp/routers/default/nearby?stops=MTASBWY%3A<STOP ID BODY (stopsForRoute)>&apikey=Z276E3rCeTzOQEoBPPN4JCEc6GfvdnYE

## Original Plan 
  1. Figure out how to scrape data from the MTA service status webpage
  2. Figure out how to work with an SQL database
  3. Go to MTA service status website and get current status of A train
  4. Allow the program to run in the background and log persistent history of A train
  5. Create interface to view history of A train
  6. Create interface to guess the (non-delayed) future behavior of the A train