

// external includes
#include <sqlite3.h>

#include <common.h>

// nyc-subway-tracker includes
#include <tracker_sqlite.h>

#define FILENAME "tracker_sqlite.cpp"
#define DEBUG_SQLITE_ZSQL 0
#define DEBUG_SQLITE_DUP 0

namespace sqlite {

std::string sqlite_statement_buf;
std::mutex test_mutex;

int reserveSqliteStatementBuf(size_t n) {
    sqlite_statement_buf.reserve(n);
    return 0;
}

sqlite3* openDB(const std::string& db_name) {
    sqlite3* db = nullptr;

    // make sure that the filename is valid (file is openable)
    std::ofstream file(db_name);
    if (!file)
        common::panic(FILENAME, "open_db", "cannot open file");
    file.close();

    if (sqlite3_open(db_name.data(), &db))
        common::panic(FILENAME, "open_db", "sqlite3_open error");

    std::string zSql = "PRAGMA journal_mode = " + constant::JOURNAL_MODE;
    if (sqlite3_exec(db, zSql.data(), nullptr, nullptr, nullptr))
        common::panic(FILENAME, "open_db", "PRAGMA journal_mode error");

    return db;
}

int createNewTable(const SqliteEnv& env, const Table& table) {
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
    writezSql(env, zSql, "create_new_table");
        
    return 0;
}

int insertRow(const SqliteEnv& env, const Table& table, const std::vector<std::string>& data) {
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
    writezSql(env, zSql, "insert_row");

    return 0;
}

int deleteRow(const SqliteEnv& env, const Table& table, const std::string& data) {
    std::string primaryKey = table.columns[0].first;
    std::string zSql = "DELETE FROM " + table.name + "WHERE " +
        primaryKey + "=" + "\"" + data[0] + "\"" + ";\n"; 
    // DELETE FROM <tablename> WHERE <primarykey> = "<data[0]>";
    
    // executes prepared zSql statement
    writezSql(env, zSql, "delete_row");

    return 0;
}

int getRow(const SqliteEnv& env, const Table& table, const std::vector<std::string>& data) {
    std::string primaryKey = table.columns[0].first;
    std::string zSql = "SELECT * FROM " + table.name + " WHERE " +
        primaryKey + " = " + "\"" + data[0] + "\"" + ";\n";
    // SELECT * from <tablename> WHERE <primarykey> = "<data[0]>";
    
    // executes prepared zSql statement
    // TODO: set up callback function stuff
    writezSql(env, zSql, "get_row");

    return 0;
}

int writezSql(const SqliteEnv& env, const std::string& zSql, const std::string& funcname) {
    if (env.db == nullptr || zSql.empty() || funcname.empty())
        common::panic(FILENAME, funcname + ", writezSql", "args");

    if (env.mutex != nullptr) {
        std::lock_guard<std::mutex> mutex(test_mutex);
        sqlite_statement_buf.append(zSql);
    } else {
        sqlite_statement_buf.append(zSql);
    }

    return 0;
}


// executes prepared zSql statement and checks arguments
int execzSql(
    const SqliteEnv& env, 
    const std::string& funcname, 
    int (*callback)(void*,int,char**,char**), 
    void* cbarg) {

    if (env.db == nullptr || sqlite_statement_buf.empty() || funcname.empty())
        common::panic(FILENAME, funcname + ", execzSql", "args");

    // print out statement if debug on
    if (DEBUG_SQLITE_ZSQL) 
        std::cerr << "[debug] [" << funcname << "] zSql: \n" << sqlite_statement_buf << std::endl;

    // execute sqlite3 statement
    int ret;
    char* errmsg = nullptr;
    if ( 
        (ret = sqlite3_exec(env.db, sqlite_statement_buf.data(), callback, cbarg, &errmsg)) &&
        ret != 19
    ) {
        std::string strerrmsg;
        if (errmsg != nullptr) {
            strerrmsg = std::string(errmsg);
            sqlite3_free(errmsg);
        }

        std::cerr << "[debug] zSql: \n" << sqlite_statement_buf << std::endl;
        common::panic(FILENAME, funcname + ", execzSql", 
            strerrmsg + " " + std::to_string(ret));
    }

    // SQL duplicate entry error
    // don't panic, just make a note of it in stdout if debug on
    if (DEBUG_SQLITE_DUP && ret == 19)
        std::cerr << "[debug] [" << funcname << "] duplicate entry: " << sqlite_statement_buf << std::endl;

    return 0;
}

}