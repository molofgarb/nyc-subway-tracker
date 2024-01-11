#ifndef TRACKER_SQLITE_H
#define TRACKER_SQLITE_H

// external includes
#include <sqlite3.h>

#include <common.h>

namespace sqlite {

    extern std::string sqlite_statement_buf;

    // reserves memory for statement buffer
    int reserveSqliteStatementBuf(size_t n);

    sqlite3* openDB(const std::string& db_name); 

    // wrappers for basic sqlite functions
    int createNewTable(const SqliteEnv& env, const Table& table);
    int insertRow(const SqliteEnv& env, const Table& table, const std::vector<std::string>& data);
    int deleteRow(const SqliteEnv& env, const Table& table, const std::string& data);
    int getRow(const SqliteEnv& env, const Table& table, const std::string& data);

    // writes an sqlite statement to the buffer
    // note that the validity of the statement is not checked
    // this is multithreadable provided that a mutex is given in env
    int writezSql(
        const SqliteEnv& env, 
        const std::string& zSql = "", 
        const std::string& funcname = "");

    // executes all sqlite statements stored in sqlite_statement_buf
    int execzSql(
        const SqliteEnv& env, 
        const std::string& funcname = "", 
        int (*callback)(void*,int,char**,char**) = nullptr, 
        void* cbarg = nullptr);
        // 2nd arg to callback is num of result columns, 3rd arg is an array of strings
        // about the type of each result, 4th arg is an array of results as strings
}

#endif