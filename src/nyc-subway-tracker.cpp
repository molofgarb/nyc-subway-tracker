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

int parseArgs(int argc, char* argv[], std::map<std::string, int>& options);
int execute();

// enums for specifying option/argument type
enum {
    INTERACTIVE,
    SILENT,
    NUM_SNAPSHOTS,
    TIME_BT_SNAPSHOTS
};

int main(int argc, char* argv[]) {
    std::map<std::string, int> options{
        {"mode", 0},
        {"snapshots", 0},
        {"time_bt_snapshots", 0}
    };

    if (parseArgs(argc, argv, options))
        common::panic(FILENAME, "main");

    size_t snapshots_num = 1;
    std::string sleep_time_str = "1";
    std::chrono::minutes sleep_time(std::stoul(sleep_time_str));

    //initialize db and have db point to it
    sqlite3* db = tracker::snapshot_db_initialize(); 

    // subway object that holds last system check status
    // this will take some time but not much compute
    Subway subway; 

    // print header
    std::cout << "Performing " << snapshots_num << " updates with " << 
        sleep_time_str << " between each update." << std::endl;

    // perform updates and snapshots
    for (size_t i = 0; i < snapshots_num; i++) {
        std::cout << "Starting Update #" << i + 1 << "..." << std::endl;
        subway.update(); //update subway object -- get current system status
        
        std::cout << "Starting Snapshot #" << i + 1 << "..." << std::endl;
        tracker::snapshot(subway, db); //log current system status in db

        if (i != snapshots_num - 1) { //sleep if not last snapshot
            std::cout << "Sleeping for " << sleep_time_str << " minutes..." << std::endl;
            std::this_thread::sleep_for(sleep_time); //wait for 1 min before next update
        }
    }

    std::cout << "All snapshot data can be found in " + constant::DB_NAME << std::endl;
    
    // std::cout << common::formatTime() << std::endl;
    // <DEBUG>
    if (DEBUG_SUBWAYOUT) {
        std::ofstream file("subway.txt");
        file << subway << std::endl;
        file.close();
    }
}

// =============================================================================

int parseArgs(int argc, char* argv[], std::map<std::string, int>& options) {
    // argument mapping
    std::map<std::string, int> argsmap{
        {"-i", INTERACTIVE},
        {"-s", SILENT},
        {"-n", NUM_SNAPSHOTS},
        {"-t", TIME_BT_SNAPSHOTS}
    };

    // flags for argument checking
    // "mode" : interactive or silent
    // "snapshots" : number of snapshots to take
    // "time_bt_snapshots" : time between snapshots
    std::set<std::string> flags;

    // help, display only if it is the only argument, then kill process
    if ( argc == 2 && (!strcmp(argv[1], "--help") || !strcmp(argv[1], "-h")) ) {
        std::cout << "Usage: " << constant::LOGGER_NAME << " [OPTIONS] [DB_FILE]" << std::endl;
        std::cout << "Options:" << std::endl;
        std::cout << "\t" << "-i\t" << "interactive mode (default)" << std::endl;
        std::cout << "\t" << "-s\t" << "silent mode" << std::endl;
        std::cout << "\t" << "-n\t" << "number of snapshots to take" << std::endl;
        std::cout << "\t" << "-t\t" << "time between snapshots in minutes" << std::endl;
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
        std::cout << "the first argument is " << argv[i] << std::endl;
        switch (argsmap[argv[i]]) {
            // interactive mode
            case (INTERACTIVE):
                // make sure that a duplicate argument has not been made
                if (flags.find("mode") != flags.end())      goto console_switch_default;

                options["mode"] = INTERACTIVE;
                flags.insert("mode");
                break;

            // silent mode -- default options used if not provided in args
            case (SILENT):
                // make sure that a duplicate argument has not been made
                if (flags.find("mode") != flags.end())      goto console_switch_default;

                options["mode"] = SILENT;
                flags.insert("mode");
                break;

            // number of snapshots to take
            case (NUM_SNAPSHOTS):
                // make sure that a duplicate argument has not been made
                if (flags.find("snapshots") != flags.end()) goto console_switch_default;

                // make sure that a next argument is provided and that it is right type
                if (++i == argc)                            goto console_switch_default;
                if (atoi(argv[i]) == 0)                     goto console_switch_default;

                options["snapshots"] = atoi(argv[i]);
                flags.insert("snapshots");
                break;

            // time between snapshots
            case (TIME_BT_SNAPSHOTS):
                // make sure that a duplicate argument has not been made
                if (flags.find("time_bt_snapshots") != flags.end()) goto console_switch_default;

                // make sure that a next argument is provided and that it is right type
                if (++i == argc)                                    goto console_switch_default;
                if (atoi(argv[i]) == 0)                             goto console_switch_default;

                options["time_bt_snapshots"] = atoi(argv[i]);
                flags.insert("time_bt_snapshots");
                break;

            console_switch_default:
            default:
                std::cout << "invalid argument: " << argv[i] << std::endl;
                exit(1);
        }
    }

    return 0;
}