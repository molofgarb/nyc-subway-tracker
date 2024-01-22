#include <pch.h>

// external includes
#include <sqlite3.h>

// nyc-subway-tracker includes
#include <tracker_sqlite.h>

#define DEBUG_SQLITE_DUMP_ZSQL      (0)
#define DEBUG_SQLITE_ERR_DUMP_ZSQL  (0)
#define DEBUG_SQLITE_DUP            (0)

const std::string TSqlite::JOURNAL_MODE = "OFF";
const std::vector<std::string> TSqlite::PLACEHOLDER_COL{"placeholder"};

// =============================================================================

TSqlite::TSqlite(const std::string& db_name, time_t time): time(time) {
    // make sure that the filename is valid (file is openable)
    std::ofstream file(db_name, std::ios_base::app);
    if (!file)
        common::panic("cannot open file");
    file.close();

    // open the db
    if (sqlite3_open(db_name.data(), &db))
        common::panic("sqlite3_open error");

    // set journalling mode for the db
    std::string zSql = "PRAGMA journal_mode = " + JOURNAL_MODE;
    if (sqlite3_exec(db, zSql.data(), nullptr, nullptr, nullptr))
        common::panic("PRAGMA journal_mode error");
}

TSqlite::~TSqlite() { execStatements(); }

int TSqlite::reserveSqliteStatementBuf(size_t n) {
    statement_buf.reserve(n);
    return 0;
}

int TSqlite::createNewTable(const Table& table) {
    std::string zSql = "CREATE TABLE IF NOT EXISTS " + table.name + "(\n";
    for (size_t i = 0; i < (table.columns).size(); i++) { //name, data type
        auto name = (table.columns)[i];
        zSql.append(
            "\"" + name + "\"" + ' ' + "\"TEXT\"" + 
            ((i == 0) ? " PRIMARY KEY" : "") + 
            ((i != (table.columns).size() - 1) ? ",\n" : ");\n")
        );
    }
    // CREATE TABLE <tablename>(
    //     "<name0>" "<type0>" PRIMARY KEY, 
    //     "<name1>" "<type1>");

    // executes prepared zSql statement
    writezSql(zSql);
        
    return 0;
}

int TSqlite::insertRow(const std::string& tablename, const std::vector<std::string>& data) {
    std::string zSql = "INSERT OR IGNORE INTO " + tablename + " VALUES (";

    //add data info
    for (size_t i = 0; i < data.size(); i++) { 
        zSql.append("\"" + data[i] + "\"");
        if (i != data.size() - 1) zSql.append(",");
    }
    zSql.append(");\n");
    // INSERT INTO <tablename>(<columnname1>,<columnname2>)
    // VALUES (<data[0]>,<data[1]>);

    // executes prepared zSql statement
    writezSql(zSql);

    return 0;
}

int TSqlite::deleteRow(const Table& table, const std::string& key) {
    execzSql();

    std::string zSql = "DELETE FROM " + table.name + " WHERE " +
        table.columns[0] + " = " + "\"" + key + "\"" + ";\n"; 
    // DELETE FROM <tablename> WHERE <primarykey> = "<data[0]>";
    // the delete will be okay even if the values dont exist
    
    // executes prepared zSql statement
    writezSql(zSql);

    return 0;
}

int TSqlite::deleteTable(const Table& table, const std::string& key) {
    // PART 1: go into key and perform drop tables on every row (which are tables) 
    // that is inside key. before we drop the table, we create the table if not 
    // exists so that we don't error out if we try to drop a leaf table

    // make the table just in case if it doesn't exist (leaf row)
    Table leafkeytable(key, PLACEHOLDER_COL);
    createNewTable(leafkeytable);

    // get the columns and rows of the key table (useful if not leaf)
    // execzSql is run to update the table in getCols
    std::vector<std::string> keytable_cols;
    std::vector<std::vector<std::string>> keytable_rows;
    getCols(key, keytable_cols);
    getAllRows(key, keytable_rows);

    Table keytable(key, keytable_cols);

    // in the base case (leaf), temptable_rows will have no entries, so this
    // will not continue the recursive callchain
    for (const auto& row : keytable_rows)
        deleteTable(keytable, row[0]);

    // PART 2: delete all rows in key, which is fine to do since all rows (tables) 
    // have been dropped. this should not error out because it returns fine if
    // there are no rows in key table
    std::string zSql = "DELETE FROM " + key + ";\n";
    writezSql(zSql);

    // PART 3: delete the table that key points to. if the table doesn't exist,
    // this still shouldn't error out
    zSql = "DROP TABLE IF EXISTS " + key + ";\n";
    writezSql(zSql);

    // PART 4: delete the row containing key from the Table table. this should
    // not error out because we know that key exists.
    deleteRow(table, key);

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
    catch (std::exception& e) { common::panic("static_cast failed"); }
    dbptr->callback(cols, data);

    return 0;
} 
}
// gets a single row's columns and returns it. the row that is acquired is the
// one whose primary key matches the given key
int TSqlite::getRow(
    const Table& table, 
    const std::string& key, 
    std::vector<std::vector<std::string>>& data) {

    execzSql();
    std::string primaryKey = table.columns[0];
    std::string zSql = "SELECT * FROM " + table.name + " WHERE " +
        primaryKey + " = " + "\"" + key + "\"" + ";\n";
    // SELECT * from <tablename> WHERE <primarykey> = "<data[0]>";

    std::lock_guard lock(read_mutex);

    // puts this new statement in the buffer and executes it with our callback function
    // execzSql will execute zSql atomically
    execzSql(zSql, &C_getRowCallback);

    // copy read buffer into data
    data = read_buf;
    read_buf.clear();

    return 0;
}

// does the same thing as getRow, but Sqlite statement is modified to get all
// rows and callback is called on all rows
int TSqlite::getAllRows(
    const std::string& tablename, 
    std::vector<std::vector<std::string>>& data) {

    execzSql();
    std::string zSql = "SELECT * FROM " + tablename + ";";
    // SELECT * from <tablename>;

    std::lock_guard lock(read_mutex);

    // puts this new statement in the buffer and executes it with our callback function
    // execzSql will execute zSql atomically
    execzSql(zSql, &C_getRowCallback);

    // copy read buffer into data (read_buf may be empty if table empty)
    data = read_buf;
    read_buf.clear();

    return 0;

}

extern "C" {
static int C_getColsCallback(
    void* TSqlite_ptr, 
    int cols, 
    char** data __attribute__((unused)), 
    char** colnames) {

    // try to cast the TSqlite object
    TSqlite* dbptr;
    try { dbptr = static_cast<TSqlite*>(TSqlite_ptr); }
    catch (std::exception& e) { common::panic("static_cast failed"); }
    dbptr->callback(cols, colnames);

    return 0;
} 
}
int TSqlite::getCols(
    const std::string& tablename,
    std::vector<std::string>& data) {

    execzSql();
    std::string zSql = "SELECT * FROM " + tablename + ";";
    // SELECT * from <tablename>;

    std::lock_guard lock(read_mutex);

    // puts this new statement in the buffer and executes it with our callback function
    // execzSql will execute zSql atomically
    execzSql(zSql, &C_getColsCallback);

    // copy read buffer into data if read buffer isnt empty (
    if (!read_buf.empty()) {
        data = read_buf[0];
        read_buf.clear();
    }

    return 0;
}

// calls execzSql without allowing caller to pass a custom callback function
int TSqlite::execStatements() {
    return execzSql();
}

int TSqlite::writezSql(const std::string& zSql, const std::string& funcname) {
    if (db == nullptr || zSql.empty() || funcname.empty())
        common::panic("args", std::string(funcname) + ", writezSql");

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
    if (DEBUG_SQLITE_DUMP_ZSQL) 
        std::cerr << "[debug] [" << funcname << "] zSql: \n" << statement_buf << std::endl << std::endl;

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

        if (DEBUG_SQLITE_ERR_DUMP_ZSQL)
            std::cerr << "[error debug] zSql: \n" << statement_buf << std::endl;
        common::panic(strerrmsg + " " + std::to_string(ret), __builtin_FILE(),
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

int TSqlite::callback(int cols, char** data) {
    // transfer data to row, and then put row in the read_buf table
    std::vector<std::string> row;
    for (int j = 0; j < cols; j++)
        row.emplace_back(data[j]);
    read_buf.emplace_back(row);
    return 0;
}
