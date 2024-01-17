#ifndef TRACKER_SQLITE_H
#define TRACKER_SQLITE_H

#include <pch.h>

// external includes
#include <sqlite3.h>

// represents a SQLite3 database
// data is stored in a hierarchical table format:
//   tables have entries that point to other tables, and leaf tables hold data
class TSqlite {
public:
    // represents the environment that a sqlite-related function executes in
    // think of it like an extended argument list
    TSqlite(
        const std::string& db_name, 
        time_t time = std::time(nullptr));
    ~TSqlite();

    time_t getTime() const { return time; }
    const time_t* getTimePtr() const { return &time; }

    // reserves memory for statement buffer
    int reserveSqliteStatementBuf(size_t n);

    // opens the database in append mode with the filename db_name
    int openDB(const std::string& db_name); 

    // creates a new, hanging table. nothing initally points to this table
    int createNewTable(const Table& table);

    // inserts a row of data into a table, the first col in the row holds the
    // primary key of that row
    int insertRow(const std::string& tablename, const std::vector<std::string>& data);

    // =========================================================================

    // deletes a row with a matching primary key from the table.
    int deleteRow(const Table& table, const std::string& key);

    // deletes the table that this primary key points to (if applicable) and 
    // all tables under that table, recursively. this also recursively deletes 
    // all tables that the primary keys (col 1) of this table points to.
    // !! table is the table that contains key !!
    int deleteTable(const Table& table, const std::string& key);

    // =========================================================================

    // gets a row matching a key in a table, where key is the primary key of
    // the row that is acquired
    int getRow(
        const Table& table, 
        const std::string& key, 
        std::vector<std::vector<std::string>>& data);

    // gets all rows in a table
    // this function also sues getRowCallback
    int getAllRows(
        const std::string& tablename, 
        std::vector<std::vector<std::string>>& data);

    int getCols(
        const std::string& tablename,
        std::vector<std::string>& data);

    // C++ callback
    // a static function C callback is used in the source file
    int callback(int cols, char** data);

    // =========================================================================

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
    static const std::vector<std::string> PLACEHOLDER_COL;
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