1. Figure out how to scrape data from the MTA service status webpage
2. Figure out how to work with an SQL database
3. Go to MTA service status website and get current status of A train
4. Allow the program to run in the background and log persistent history of A train
5. Create interface to view history of A train
6. Create interface to guess the (non-delayed) future behavior of the A train
7. Implement for all trains

## Notes
- Only keep last 30 days of data or a certain limit -- if data is older than current date and
  the current amount of data is greater than the limit, then delete to meet the limit starting from oldest
- Make sure to scrape at not too quick of an interval that bad things happen, but enough so that every train is captured
- Should be able to create a table to uniquely identify a train as it progresses through stations

main: 
qeqy84JE7hUKfaI0Lxm2Ttcm6ZA0bYrP - alerts (for ALL forms of transport - dynamic)
Z276E3rCeTzOQEoBPPN4JCEc6GfvdnYE - routes (for ALL forms of transport - static)
qeqy84JE7hUKfaI0Lxm2Ttcm6ZA0bYrP - stopsForRoute (after clicking on a train - static)
Z276E3rCeTzOQEoBPPN4JCEc6GfvdnYE - nearby (after clicking on a stop - dynamic)