#include <sqlite3.h>

#include <common.h>
    using namespace std::chrono_literals;

// nyc-subway-tracker includes
#include <tracker.h>
#include <subway.h>
#include <line.h>
#include <station.h>

#define FILENAME "nyc-subway-tracker.cpp"

#define DEBUG_SUBWAYOUT 0

// parses cmdline arguments passed to pain and populates options and db_filename
int parseArgs(int argc, char* argv[], std::unordered_map<int, int>& options, std::string& db_filename);

// gets values for options and db_filename interactively
int interactive(std::unordered_map<int, int>& options, std::string& db_filename);

// formatted time prefix for each message for log-like format
std::string timestr() { return "<" + common::formatTime() + "> "; }

// reports the invalid argument that was given and then terminates process
int invalid_arg(char* arg) { std::cout << "invalid argument: " << arg << std::endl; exit(1); }

// enums for specifying option/argument type
enum {
    // options
    MODE,

    // argument types for options
    INTERACTIVE,
    SILENT,
    NUM_SNAPSHOTS,
    TIME_BT_SNAPSHOTS
};

int main(int argc, char* argv[]) {
    time_t starttime = std::time(nullptr);
    std::unordered_map<int, int> options{
        {MODE, SILENT},
        {NUM_SNAPSHOTS, 1},
        {TIME_BT_SNAPSHOTS, 1}
    };
    std::string db_filename = constant::DB_NAME;

    // parse arguments into options
    if (parseArgs(argc, argv, options, db_filename))
        common::panic(FILENAME, "main");

    // interactively get options
    if (options[MODE] == INTERACTIVE) interactive(options, db_filename);

    std::chrono::minutes sleep_time(options[NUM_SNAPSHOTS]);

    //initialize db and have db point to it
    std::cout << timestr() << "Initializing database..." << std::endl;
    sqlite3* db = tracker::snapshot_db_initialize();
    std::cout << timestr() << "Initialized database.\n" << std::endl;

    // subway object that holds last system check status
    // this will take some time but not much compute
    std::cout << timestr() << "Initializing subway..." << std::endl;
    Subway subway;
    std::cout << timestr() << "Initialized subway.\n" << std::endl;

    // print header
    std::cout << "Performing " << options[NUM_SNAPSHOTS] << " updates with " <<
        options[TIME_BT_SNAPSHOTS] << " minute between each update.\n" << std::endl;

    // perform updates and snapshots
    for (int i = 0; i < options[NUM_SNAPSHOTS]; i++) {

        //update subway object -- get current system status
        std::cout << timestr() << "Starting Update #" << i + 1 << "..." << std::endl;
        subway.update();
        std::cout << timestr() << "Finished Update #" << i + 1 << ".\n" << std::endl;

        //log current system status in db
        std::cout << timestr() << "Starting Snapshot #" << i + 1 << "..." << std::endl;
        tracker::snapshot(subway, db);
        std::cout << timestr() << "Finished Snapshot #" << i + 1 << ".\n" << std::endl;

        if (i != options[NUM_SNAPSHOTS] - 1) { //sleep if not last snapshot
            std::cout << timestr() << "Sleeping for " << options[TIME_BT_SNAPSHOTS] << " minutes...\n" << std::endl;
            std::this_thread::sleep_for(sleep_time); //wait for 1 min before next update
        }
    }

    std::cout << "All snapshot data can be found in " << db_filename << std::endl;
    std::cout << "Time elapsed: " << std::time(nullptr) - starttime << " seconds." << std::endl;

    // std::cout << common::formatTime() << std::endl;
    // <DEBUG>
    if (DEBUG_SUBWAYOUT) {
        std::ofstream file("subway.txt");
        file << subway << std::endl;
        file.close();
    }
}

// =============================================================================

int parseArgs(int argc, char* argv[], std::unordered_map<int, int>& options, std::string& db_filename) {
    // argument mapping
    std::unordered_map<std::string, int> argsmap{
        {"-i", INTERACTIVE},
        {"-s", SILENT},
        {"-n", NUM_SNAPSHOTS},      // this takes a positive integer after
        {"-t", TIME_BT_SNAPSHOTS}   // this takes a positive integer after
    };

    // flags for argument checking
    // "mode" : interactive or silent
    // "snapshots" : number of snapshots to take
    // "time_bt_snapshots" : time between snapshots
    std::unordered_set<int> flags;

    // help, display only if it is the only argument, then kill process
    if ( argc == 2 && (!strcmp(argv[1], "--help") || !strcmp(argv[1], "-h")) ) {
        std::cout << "Usage: " << constant::LOGGER_NAME << " [OPTIONS] [DB_FILE]" << std::endl;
        std::cout << "Options:" << std::endl;
        std::cout << "  " << "-i\t" << "interactive mode" << std::endl;
        std::cout << "  " << "-s\t" << "silent mode (default)" << std::endl;
        std::cout << "  " << "-n\t" << "number of snapshots to take" << std::endl;
        std::cout << "  " << "-t\t" << "time between snapshots in minutes" << std::endl;
        std::cout << std::endl;

        std::cout <<
            "DB_file: name of the file that the log DB entries should be" <<
            "stored in. The default is \"" <<
            constant::DB_NAME <<
            "\"."
        << std::endl;
        exit(0);
    }

    for (int i = 1; i < argc; i++) {
        switch (argsmap[argv[i]]) {
        // interactive mode
        case (INTERACTIVE):
            // make sure that a duplicate argument has not been made
            if (flags.find(MODE) != flags.end())      invalid_arg(argv[i]);

            options[MODE] = INTERACTIVE;
            flags.insert(MODE);
            break;

        // silent mode -- default options used if not provided in args
        case (SILENT):
            // make sure that a duplicate argument has not been made
            if (flags.find(MODE) != flags.end())      invalid_arg(argv[i]);

            options[MODE] = SILENT;
            flags.insert(MODE);
            break;

        // number of snapshots to take
        case (NUM_SNAPSHOTS):
            // make sure that a duplicate argument has not been made
            if (flags.find(NUM_SNAPSHOTS) != flags.end())   invalid_arg(argv[i]);

            // make sure that a next argument is provided and that it is right type
            if (++i == argc)                                invalid_arg(argv[i - 1]);
            if (atoi(argv[i]) == 0)                         invalid_arg(argv[i]);

            options[NUM_SNAPSHOTS] = atoi(argv[i]);
            flags.insert(NUM_SNAPSHOTS);
            break;

        // time between snapshots
        case (TIME_BT_SNAPSHOTS):
            // make sure that a duplicate argument has not been made
            if (flags.find(TIME_BT_SNAPSHOTS) != flags.end())   invalid_arg(argv[i]);

            // make sure that a next argument is provided and that it is right type
            if (++i == argc)                                    invalid_arg(argv[i - 1]);
            if (atoi(argv[i]) == 0)                             invalid_arg(argv[i]);

            options[TIME_BT_SNAPSHOTS] = atoi(argv[i]);
            flags.insert(TIME_BT_SNAPSHOTS);
            break;

        default:
            // if database file name, last arg
            // we just check that it's not an option, it can have any name format
            if ( (i == argc - 1) && (argv[i][0] != '-') )
                db_filename = std::string(argv[i]);
            else
                invalid_arg(argv[i]);
            break;
        }
    }

    return 0;
}

int interactive(std::unordered_map<int, int>& options, std::string& db_filename) {
    std::string buf;

    // get number of snapshots
    std::cout << "Number of Snapshots to take (default: 1): ";
    std::getline(std::cin, buf);

    // if buf is non-default
    if (!(buf == "")) {
        // verify input to be positive integer
        int bufint = atoi(buf.data());
        if (bufint < 1) invalid_arg(buf.data());

        options[NUM_SNAPSHOTS] = bufint;
    }

    // get time between snapshots
    std::cout << "Time in minutes between snapshots to wait (default: 1): ";
    std::getline(std::cin, buf);

    // if buf is non-default
    if (!(buf == "")) {
        // verify input to be positive integer
        int bufint = atoi(buf.data());
        if (bufint < 1) invalid_arg(buf.data());

        options[TIME_BT_SNAPSHOTS] = bufint;
    }

    // get name of database file
    std::cout << "Database file name (default: nyc-subway-tracker.db): ";
    std::getline(std::cin, buf);

    // if buf is non-default
    if (!(buf == "")) {
        // verify input doesn't have hyphen as first char for silent mode parity
        if (buf[0] != '-')
            db_filename = buf;
    }

    return 0;
}
