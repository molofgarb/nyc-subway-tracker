

// external includes
#include <sqlite3.h>

#include <common.h>

// nyc-subway-tracker includes
#include <tracker_sqlite.h>

#define FILENAME "tracker_sqlite.cpp"
#define DEBUG_SQLITE_ZSQL 0
#define DEBUG_SQLITE_DUP 1

namespace sqlite {

sqlite3* open_db(const std::string& db_name) {
    sqlite3* db = nullptr;
    if (sqlite3_open(db_name.data(), &db))
        common::panic(FILENAME, "open_db", "open error");
    return db;
}

sqlite3* create_new_table(sqlite3* db, const Table& table) {
    std::string zSql = "CREATE TABLE IF NOT EXISTS " + table.name + "(\n";
    for (size_t i = 0; i < (table.columns).size(); i++) { //name, data type
        auto name = (table.columns)[i].first;
        auto type = (table.columns)[i].second;
        zSql.append(
            "\"" + name + "\"" + ' ' + "\"" + type + "\"" + 
            ((i == 0) ? " PRIMARY KEY" : "") + 
            ((i != (table.columns).size() - 1) ? ",\n" : ");")
        );
    }
    // CREATE TABLE <tablename>(
    //     "<name0>" "<type0>" PRIMARY KEY, 
    //     "<name1>" "<type1>");

    // executes prepared zSql statement
    exec_zSql(db, zSql, "create_new_table");
        
    return db;
}

sqlite3* insert_row(sqlite3* db, const Table& table, const std::vector<std::string>& data) {
    std::string zSql = "INSERT INTO " + table.name + "(";

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
    zSql.append(");");
    // INSERT INTO <tablename>(<columnname1>,<columnname2>)
    // VALUES (<data[0]>,<data[1]>);

    // executes prepared zSql statement
    exec_zSql(db, zSql, "insert_row");

    return db;
}

sqlite3* delete_row(sqlite3* db, const Table& table, const std::string& data) {
    std::string primaryKey = table.columns[0].first;
    std::string zSql = "DELETE FROM " + table.name + "WHERE " +
        primaryKey + "=" + "\"" + data[0] + "\"" + ";"; 
    // DELETE FROM <tablename> WHERE <primarykey> = "<data[0]>";
    
    // executes prepared zSql statement
    exec_zSql(db, zSql, "delete_row");

    return db;
}

sqlite3* get_row(sqlite3* db, const Table& table, const std::vector<std::string>& data) {
    std::string primaryKey = table.columns[0].first;
    std::string zSql = "SELECT * FROM " + table.name + " WHERE " +
        primaryKey + " = " + "\"" + data[0] + "\"" + ";";
    // SELECT * from <tablename> WHERE <primarykey> = "<data[0]>";
    
    // executes prepared zSql statement
    // TODO: set up callback function stuff
    exec_zSql(db, zSql, "get_row");

    return db;
}

// executes prepared zSql statement and checks arguments
void exec_zSql(
    sqlite3* db, const std::string& zSql, const std::string& funcname, 
    int (*callback)(void*,int,char**,char**), void* cbarg
) {
    if (db == nullptr || zSql.empty() || funcname.empty())
        common::panic(FILENAME, funcname + ", exec_zSql", "args");

    // print out statement if debug on
    if (DEBUG_SQLITE_ZSQL) 
        std::cerr << "<debug> <" << funcname << "> zSql: \n" << zSql << std::endl;

    // execute sqlite3 statement
    int ret;
    char* errmsg = nullptr;
    if ( 
        (ret = sqlite3_exec(db, zSql.data(), callback, cbarg, &errmsg)) &&
        ret != 19
    ) {

        std::string strerrmsg;
        if (errmsg != nullptr) {
            strerrmsg = std::string(errmsg);
            sqlite3_free(errmsg);
        }

        std::cerr << "<debug> zSql: \n" << zSql << std::endl;
        common::panic(FILENAME, funcname + ", exec_zSql", 
            strerrmsg + " " + std::to_string(ret));
    }

    // SQL duplicate entry error
    // don't panic, just make a note of it in stdout if debug on
    if (DEBUG_SQLITE_DUP && ret == 19)
        std::cerr << "<debug> <" << funcname << "> duplicate entry: " << zSql << std::endl;
}

}