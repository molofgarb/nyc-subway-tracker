#ifndef TRACKER_SQLITE_H
#define TRACKER_SQLITE_H

#include <pch.h>

// external includes
#include <sqlite3.h>

class TSqlite {
public:
    // represents the environment that a sqlite-related function executes in
    // think of it like an extended argument list
    TSqlite(
        const std::string& db_name, 
        time_t time = std::time(nullptr));

    time_t getTime() const { return time; }
    const time_t* getTimePtr() const { return &time; }

    // reserves memory for statement buffer
    int reserveSqliteStatementBuf(size_t n);

    int openDB(const std::string& db_name); 

    // wrappers for basic sqlite functions
    int createNewTable(const Table& table);
    int insertRow(const Table& table, const std::vector<std::string>& data);
    int deleteRow(const Table& table, const std::string& key);

    int getRowCallback(int cols, char** data);

    // gets a row matching a key in a table, where key is the primary key of
    // the row that is acquired
    int getRow(
        const Table& table, 
        const std::string& key, 
        std::vector<std::vector<std::string>>& data);

    // gets all rows in a table
    int getAllRows(
        const Table& table, 
        std::vector<std::vector<std::string>>& data);

    // execcutes all statements in statement_buf
    int execStatements();

private:
    sqlite3* db;
    std::mutex db_mutex;

    time_t time;

    std::string statement_buf;

    std::vector<std::vector<std::string>> read_buf;
    std::mutex read_mutex;

    // writes an sqlite statement to the buffer
    // note that the validity of the statement is not checked
    // this is multithreadable provided that a mutex is given in env
    int writezSql(
        const std::string& zSql = "", 
        const std::string& funcname = __builtin_FUNCTION());

    // executes all sqlite statements stored in sqlite_statement_buf
    int execzSql(
        const std::string& zSql = "", 
        int (*callback)(void*, int, char**, char**) = nullptr,
        const std::string& funcname = __builtin_FUNCTION());
        // 2nd arg to callback is num of result columns, 3rd arg is an array of strings
        // about the type (ex. TEXT) of each result, 4th arg is an array of results as strings

    // these are the constants:
    static const std::string JOURNAL_MODE;
};

    // For each snapshot:
    // SNAPSHOTS (one)
    //          |--> SUBWAY (one)
    //                     |--> STATIONS (mul)
    //                                  |--> STATION_DATA (data)
    //                     |--> LINES (mul)
    //                                  |--> STATIONS (mul) 
    //                                               |--> STATION_DATA (data)

#endif