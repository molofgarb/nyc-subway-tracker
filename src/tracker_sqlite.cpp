#include <pch.h>

// external includes
#include <sqlite3.h>

// nyc-subway-tracker includes
#include <tracker_sqlite.h>

#define FILENAME __builtin_FILE()
#define DEBUG_SQLITE_ZSQL 0
#define DEBUG_SQLITE_DUP 0

const std::string TSqlite::JOURNAL_MODE = "OFF";

int TSqlite::reserveSqliteStatementBuf(size_t n) {
    statement_buf.reserve(n);
    return 0;
}

TSqlite::TSqlite(const std::string& db_name, time_t time): time(time) {
    // make sure that the filename is valid (file is openable)
    std::ofstream file(db_name, std::ios_base::app);
    if (!file)
        common::panic(FILENAME, "cannot open file");
    file.close();

    // open the db
    if (sqlite3_open(db_name.data(), &db))
        common::panic(FILENAME, "sqlite3_open error");

    // set journalling mode for the db
    std::string zSql = "PRAGMA journal_mode = " + JOURNAL_MODE;
    if (sqlite3_exec(db, zSql.data(), nullptr, nullptr, nullptr))
        common::panic(FILENAME, "PRAGMA journal_mode error");
}

int TSqlite::createNewTable(const Table& table) {
    std::string zSql = "CREATE TABLE IF NOT EXISTS " + table.name + "(\n";
    for (size_t i = 0; i < (table.columns).size(); i++) { //name, data type
        auto name = (table.columns)[i].first;
        auto type = (table.columns)[i].second;
        zSql.append(
            "\"" + name + "\"" + ' ' + "\"" + type + "\"" + 
            ((i == 0) ? " PRIMARY KEY" : "") + 
            ((i != (table.columns).size() - 1) ? ",\n" : ");\n")
        );
    }
    // CREATE TABLE <tablename>(
    //     "<name0>" "<type0>" PRIMARY KEY, 
    //     "<name1>" "<type1>");

    // executes prepared zSql statement
    writezSql(zSql, "create_new_table");
        
    return 0;
}

int TSqlite::insertRow(const Table& table, const std::vector<std::string>& data) {
    std::string zSql = "INSERT OR IGNORE INTO " + table.name + "(";

    //add table info
    for (size_t i = 0; i < (table.columns).size(); i++) { 
        zSql.append("\"" + (table.columns)[i].first + "\""); //names of rows
        if (i != (table.columns).size() - 1) zSql.append(",");
    }
    zSql.append(")\nVALUES (");

    //add data info
    for (size_t i = 0; i < data.size(); i++) { 
        zSql.append("\"" + data[i] + "\"");
        if (i != data.size() - 1) zSql.append(",");
    }
    zSql.append(");\n");
    // INSERT INTO <tablename>(<columnname1>,<columnname2>)
    // VALUES (<data[0]>,<data[1]>);

    // executes prepared zSql statement
    writezSql(zSql, "insert_row");

    return 0;
}

int TSqlite::deleteRow(const Table& table, const std::string& key) {
    std::string primaryKey = table.columns[0].first;
    std::string zSql = "DELETE FROM " + table.name + " WHERE " +
        primaryKey + " = " + "\"" + key + "\"" + ";\n"; 
    // DELETE FROM <tablename> WHERE <primarykey> = "<data[0]>";
    // the delete will be okay even if the values dont exist
    
    // executes prepared zSql statement
    writezSql(zSql, "delete_row");

    return 0;
}

// callbacks for getting rows. the C callback is needed for the SQLite C
// interface and forwards the call to getRowCallBack member function
extern "C" {
static int C_getRowCallback(
    void* TSqlite_ptr, 
    int cols, 
    char** data, 
    char** colnames __attribute__((unused))) {

    // try to cast the TSqlite object
    TSqlite* dbptr;
    try { dbptr = static_cast<TSqlite*>(TSqlite_ptr); }
    catch (std::exception& e) { common::panic(FILENAME, "static_cast failed"); }
    dbptr->getRowCallback(cols, data);

    return 0;
} 
}
int TSqlite::getRowCallback(int cols, char** data) {
    // transfer data to row, and then put row in the read_buf table
    std::vector<std::string> row;
    for (int j = 0; j < cols; j++)
        row.emplace_back(data[j]);
    read_buf.emplace_back(row);
    return 0;
}

// gets a single row's columns and returns it. the row that is acquired is the
// one whose primary key matches the given key
int TSqlite::getRow(
    const Table& table, 
    const std::string& key, 
    std::vector<std::vector<std::string>>& data) {

    std::string primaryKey = table.columns[0].first;
    std::string zSql = "SELECT * FROM " + table.name + " WHERE " +
        primaryKey + " = " + "\"" + key + "\"" + ";\n";
    // SELECT * from <tablename> WHERE <primarykey> = "<data[0]>";

    std::lock_guard lock(read_mutex);

    // puts this new statement in the buffer and executes it with our callback function
    // execzSql will execute zSql atomically
    execzSql(zSql, &C_getRowCallback);

    // copy read buffer into data
    data = read_buf;

    return 0;
}

// does the same thing as getRow, but Sqlite statement is modified to get all
// rows and callback is called on all rows
int TSqlite::getAllRows(
    const Table& table, 
    std::vector<std::vector<std::string>>& data) {

    std::string primaryKey = table.columns[0].first;
    std::string zSql = "SELECT * FROM " + table.name + ";";
    // SELECT * from <tablename>;

    std::lock_guard lock(read_mutex);

    // puts this new statement in the buffer and executes it with our callback function
    // execzSql will execute zSql atomically
    execzSql(zSql, &C_getRowCallback);

    // copy read buffer into data
    data = read_buf;

    return 0;

}

// calls execzSql without allowing caller to pass a custom callback function
int TSqlite::execStatements() {
    return execzSql();
}

int TSqlite::writezSql(const std::string& zSql, const std::string& funcname) {
    if (db == nullptr || zSql.empty() || funcname.empty())
        common::panic(FILENAME, "args", std::string(funcname) + ", writezSql");

    std::lock_guard<std::mutex> lock(db_mutex);
    statement_buf.append(zSql);

    return 0;
}

// executes prepared zSql statement and checks arguments
int TSqlite::execzSql(
    const std::string& zSql, 
    int (*callback)(void*, int, char**, char**),
    const std::string& funcname) {

    // If given a zSql and callback function, this function executes in two stages:
    // 1. Executes statements in the buffer without any callback
    // 2. Executes the statement in zSql with the callback
    // A lock on mutex is acquired before stage 1 and kept between stages and for stage 2.

    // tracks if a goto was executed
    bool goto_complete = false;



    std::lock_guard<std::mutex> lock(db_mutex);

    int ret;
    char* errmsg = nullptr;
    int (*cb)(void*, int, char**, char**) = nullptr;

    // execute sqlite3 statement
    execzSql_execute_statement:
    // print out statement if debug on
    if (DEBUG_SQLITE_ZSQL) 
        std::cerr << "[debug] [" << funcname << "] zSql: \n" << statement_buf << std::endl;
    if ( 
        (ret = sqlite3_exec(db, statement_buf.data(), cb, this, &errmsg)) &&
        ret != 19
    ) {
        // emit zSql and error message if something went wrong
        std::string strerrmsg;
        if (errmsg != nullptr) {
            strerrmsg = std::string(errmsg);
            sqlite3_free(errmsg);
        }

        std::cerr << "[debug] zSql: \n" << statement_buf << std::endl;
        common::panic(FILENAME, strerrmsg + " " + std::to_string(ret), 
            std::string(funcname) + ", execzSql");
    }

    // clear buffer since we finished executing everything
    statement_buf.clear();
    
    // return if no additional statement given or if a goto was completed
    if (zSql == "" || goto_complete) return 0;

    // I'm using a goto because it looks much nicer than doing a function subcall,
    // doing some loop stuff, or copypasting that same execution block again.

    // Add statement and then perform the zSql execution block again
    statement_buf.append(zSql);
    cb = callback;
    goto_complete = true;
    goto execzSql_execute_statement;
}
