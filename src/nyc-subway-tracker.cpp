#include <pch.h>
    using namespace std::chrono_literals;

// nyc-subway-tracker includes
#include <tracker.h>

#define FILENAME __builtin_FILE()
#define DEBUG_SUBWAYOUT 0

// parses cmdline arguments passed to pain and populates options and db_filename
int parseArgs(
    int argc, 
    char* argv[], 
    std::unordered_map<int, std::any>& options);

// gets values for options and db_filename interactively
int interactive(std::unordered_map<int, std::any>& options);

int summary(
    std::ostream& stream, 
    const std::string& db_filename,
    time_t start_time, 
    time_t end_time,
    const std::vector<time_t>& update_times,
    const std::vector<time_t>& snapshot_times);

// formatted time prefix for each message for log-like format
std::string timestr() { return "[" + common::formatTime() + "] "; }

// reports the invalid argument that was given and then terminates process
int invalidArg(char* arg) { std::cout << "invalid argument: " << arg << std::endl; exit(1); }

// enums for specifying option/argument type
enum {
    // make sure that no enums have a value of 0 because we use them in conditionals
    ZERO = 0,

    // argument types for options
    INTERACTIVE,
    SILENT,
    NUM_SNAPSHOTS,
    TIME_BT_SNAPSHOTS,
    LOGFILE,
    DBFILE,

    // values for options
    NUM_SNAPSHOTS_INF = -1,
};

// =============================================================================

int main(int argc, char* argv[]) {
    // initialize time and options
    time_t start_time = std::time(nullptr);
    std::unordered_map<int, std::any> options{
        // INTERACTIVE
        // SILENT
        {NUM_SNAPSHOTS, 1},
        {TIME_BT_SNAPSHOTS, 1},
        // LOGFILE
        {DBFILE, constant::DB_NAME}
    };

    // parse arguments into options
    if (parseArgs(argc, argv, options))
        common::panic(FILENAME, "parseArgs");

    // interactively get options
    if (options.find(INTERACTIVE) != options.end()) interactive(options);

    // if silent run mode, then set std::cout to be failed so it never outputs
    if (options.find(SILENT) != options.end()) std::cout.setstate(std::ios::failbit);

    size_t num_snapshots = std::any_cast<int>(options[NUM_SNAPSHOTS]);
    int time_bt_snapshots = std::any_cast<int>(options[TIME_BT_SNAPSHOTS]);
    std::string db_filename = std::any_cast<std::string>(options[DBFILE]);
    std::chrono::minutes sleep_time(time_bt_snapshots);

    // subway object that holds last system check status
    // this will take some time but not much compute
    std::cout << timestr() << "Initializing subway..." << std::endl;
    Subway subway;
    std::cout << timestr() << "Initialized subway.\n" << std::endl;

    // print header
    std::cout << "Performing " << num_snapshots << " updates with " <<
        time_bt_snapshots << " minutes between each update.\n" << std::endl;

    std::vector<time_t> update_times;
    std::vector<time_t> snapshot_times;
    update_times.reserve(num_snapshots);
    snapshot_times.reserve(num_snapshots);

    // perform updates and snapshots
    for (size_t i = 0; i < num_snapshots; i++) {

        // update subway object to get current system status
        std::cout << timestr() << "Starting Update #" << i + 1 << "..." << std::endl;
        update_times.emplace_back(std::time(nullptr));
        subway.update();
        update_times[i] = std::time(nullptr) - update_times[i];
        std::cout << timestr() << "Finished Update #" << i + 1 << ".\n" << std::endl;

        // log current system status in db using the subway that we just updated
        std::cout << timestr() << "Starting Snapshot #" << i + 1 << "..." << std::endl;
        snapshot_times.emplace_back(std::time(nullptr));
        tracker::snapshot(subway, db_filename);
        snapshot_times[i] = std::time(nullptr) - snapshot_times[i];
        std::cout << timestr() << "Finished Snapshot #" << i + 1 << ".\n" << std::endl;

        if (i != num_snapshots - 1) { //sleep if not last snapshot
            std::cout << timestr() << "Sleeping for " << time_bt_snapshots << " minutes..." << std::endl;
            std::this_thread::sleep_for(sleep_time); //wait for 1 min before next update
        }
        std::cout << "========================================\n" << std::endl;
    }

    time_t end_time = std::time(nullptr);

    // print a summary of the times to std::cout
    if (summary(std::cout, db_filename, start_time, end_time, update_times, snapshot_times))
        common::panic(FILENAME, "std::cout summary failed");

    // if a logfile was provided, then also print the summary of times to the logfile
    if (options.find(LOGFILE) != options.end()) {
        std::ofstream logfile(std::any_cast<std::string>(options[LOGFILE]));
        if (summary(logfile, db_filename, start_time, end_time, update_times, snapshot_times))
            common::panic(FILENAME, "logfile summary failed");
        logfile.close();
    }

    // hopefully nothing went wrong
    return 0;

    // <DEBUG>
    std::cout.setstate(std::ios::goodbit);
    // std::cout << common::formatTime() << std::endl;
    // if (DEBUG_SUBWAYOUT) {
    //     std::ofstream file("subway.txt");
    //     file << subway << std::endl;
    //     file.close();
    // }
}

// =============================================================================

int parseArgs(
    int argc, 
    char* argv[], 
    std::unordered_map<int, std::any>& options) {
        
    // argument mapping
    std::unordered_map<std::string, int> argsmap{
        {"-i", INTERACTIVE},
        {"-s", SILENT},
        {"-n", NUM_SNAPSHOTS},      // this takes a positive integer after
        {"-t", TIME_BT_SNAPSHOTS},   // this takes a positive integer after
        {"-l", LOGFILE}             // this takes a string after
    };

    // flags for argument checking
    // "mode" : interactive or silent
    // "snapshots" : number of snapshots to take
    // "time_bt_snapshots" : time between snapshots
    std::unordered_set<int> flags;

    // help, display only if it is the only argument, then kill process
    if ( argc == 2 && (!strcmp(argv[1], "--help") || !strcmp(argv[1], "-h")) ) {
        std::cout << "Usage: " << constant::LOGGER_NAME << " [OPTIONS] [DB_FILE]\n";
        std::cout << "If no options are given, then the program will take one snapshot and "\
                     "store the results in \"nyc-subway-tracker.db\" in the current directory.\n";
        std::cout << "Options:\n";
        std::cout << "  " << "-i\t" << "interactive mode, provides a prompt for each option\n"\
                     "    \tif you enter any other arguments, they will get overwritten in interactive mode\n";
        std::cout << "  " << "-s\t" << "silent mode, no output to stdout from program\n";
        std::cout << "  " << "-n [num]\t"\
                     "number of snapshots to take (leave empty for infinite)\n";
        std::cout << "  " << "-t [num]\t"\
                     "time between snapshots in minutes (leave empty for no time)\n";
        std::cout << "  " << "-l [file]\t"\
                     "file that the program log should be saved to\n";
        std::cout << "\n";
        std::cout << "DB_file: name of the file that the log DB entries should be" <<
                     "stored in. The default is \"" << constant::DB_NAME << "\".";
        std::cout << std::endl;
        exit(0);
    }

    for (int i = 1; i < argc; i++) {
        switch (argsmap[argv[i]]) {
        // interactive mode
        case (INTERACTIVE):
            // make sure that a duplicate argument has not been made
            if (flags.find(INTERACTIVE) != flags.end())      
                invalidArg(argv[i]);

            options[INTERACTIVE] = INTERACTIVE;
            flags.insert(INTERACTIVE);
            break;

        // silent mode -- default options used if not provided in args
        case (SILENT):
            // make sure that a duplicate argument has not been made
            if (flags.find(SILENT) != flags.end())      
                invalidArg(argv[i]);

            options[SILENT] = SILENT;
            flags.insert(SILENT);
            break;

        // number of snapshots to take
        case (NUM_SNAPSHOTS):
            // make sure that a duplicate argument has not been made
            if (flags.find(NUM_SNAPSHOTS) != flags.end())   
                invalidArg(argv[i]);

            // if there is no next argument or the next argument is another option
            if (++i == argc || argsmap.find(argv[i]) != argsmap.end()) {
                options[NUM_SNAPSHOTS] = NUM_SNAPSHOTS_INF;
                goto loc_case_numsnapshots_flaginsert;
            }  

            // if the number given is invalid
            if (atoi(argv[i]) == 0)                         
                invalidArg(argv[i]);

            options[NUM_SNAPSHOTS] = atoi(argv[i]);

        loc_case_numsnapshots_flaginsert:
            flags.insert(NUM_SNAPSHOTS);
            break;

        // time between snapshots
        case (TIME_BT_SNAPSHOTS):
            // make sure that a duplicate argument has not been made
            if (flags.find(TIME_BT_SNAPSHOTS) != flags.end())   
                invalidArg(argv[i]);

            // if there is no next argument or the next argument is another option
            if (++i == argc || argsmap.find(argv[i]) != argsmap.end()) {
                options[TIME_BT_SNAPSHOTS] = 0;
                goto loc_case_timebtsnapshots_flaginsert;
            }  

            // if the number given is invalid
            if (atoi(argv[i]) == 0)                         
                invalidArg(argv[i]);

            options[TIME_BT_SNAPSHOTS] = atoi(argv[i]);

        loc_case_timebtsnapshots_flaginsert:
            flags.insert(TIME_BT_SNAPSHOTS);
            break;

        case (LOGFILE): {
            // make sure that a duplicate argument has not been made
            if (flags.find(LOGFILE) != flags.end())
                invalidArg(argv[i]);

            // if there is no next argument or the next argument is another option
            if (++i == argc || argsmap.find(argv[i]) != argsmap.end())
                invalidArg(argv[--i]);

            // check if the file is valid by seeing if we can open it
            std::ofstream file(argv[i]);
            if (!file)
                invalidArg(argv[i]);
            file.close();

            options[LOGFILE] = std::string(argv[i]);

            flags.insert(LOGFILE);
            break;
        }

        default:
            // if database file name, last arg
            // we just check that it's not an option, it can have any name format
            // but we also want to check if the file has a valid name
            if (i + 1 == argc) {
                std::ofstream file(argv[i]);
                if (!file)
                    invalidArg(argv[i]);
                file.close();
                options[DBFILE] = std::string(argv[i]);
            } else
                invalidArg(argv[i]);
            break;
        }
    }

    return 0;
}

int interactive(std::unordered_map<int, std::any>& options) {
    std::string buf;
    buf.reserve(32);

    // decide if program should be silent
    buf.clear();
    std::cout << "Silent run (type any characters to enable silence, leave blank to disable): ";
    std::getline(std::cin, buf);

    // if buf is non-default
    if (!(buf == "")) {
        options[SILENT] = SILENT;
    }

    // get number of snapshots
    buf.clear();
    std::cout << "Number of Snapshots to take (default: 1): ";
    std::getline(std::cin, buf);

    // if buf is non-default
    if (!(buf == "")) {
        // verify input to be positive integer
        int bufint = atoi(buf.data());
        if (bufint < 1) invalidArg(buf.data());

        options[NUM_SNAPSHOTS] = bufint;
    }

    // get time between snapshots
    buf.clear();
    std::cout << "Time in minutes between snapshots to wait (default: 1): ";
    std::getline(std::cin, buf);

    // if buf is non-default
    if (!(buf == "")) {
        // verify input to be positive integer
        int bufint = atoi(buf.data());
        if (bufint < 1) invalidArg(buf.data());

        options[TIME_BT_SNAPSHOTS] = bufint;
    }

    // get name of log file
    buf.clear();
    std::cout << "Log file name (disabled by default): ";
    std::getline(std::cin, buf);

    // if buf is non-default
    if (!(buf == "")) {
        std::ofstream file(buf);
        if (!file)
            invalidArg(buf.data());
        file.close();

        options[LOGFILE] = buf;
    }

    // get name of database file
    buf.clear();
    std::cout << "Database file name (default: nyc-subway-tracker.db): ";
    std::getline(std::cin, buf);

    // if buf is non-default
    if (!(buf == "")) {
        std::ofstream file(buf);
        if (!file)
            invalidArg(buf.data());
        file.close();

        options[DBFILE] = buf;
    }

    std::cout << std::endl;

    return 0;
}

int summary(
    std::ostream& stream,
    const std::string& db_filename,
    time_t start_time, 
    time_t end_time,
    const std::vector<time_t>& update_times,
    const std::vector<time_t>& snapshot_times) {

    stream << "All snapshot data can be found in " << db_filename << "\n";
    stream << "Time elapsed: " << end_time - start_time << " seconds." << "\n";
    for (size_t i = 0; i < update_times.size(); i++)
        stream << "\tUpdate " << i+1 << ":\t" << update_times[i] << " seconds\tSnapshot " << i+1
            << ":\t" << snapshot_times[i] << " seconds\n";
    stream << std::endl;

    return 0;
}