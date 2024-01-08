// nyc-subway-tracker includes
#include <common.h>

#define FILENAME "common.cpp"

//Train equality operator
bool operator==(const Train& lhs, const Train& rhs) {
    return ((lhs.name == rhs.name) && (lhs.dirID == rhs.dirID));
}

//Train comparison operator (needed for std::map)
bool operator<(const Train& lhs, const Train& rhs) {
    return (lhs.name == rhs.name) ? (lhs.dirID < rhs.dirID) : (lhs.name < rhs.name);
}

// =============================================================================
namespace common {

std::string formatTime(time_t* time, int mode) {
    time_t now;

    // if a time is not supplied, then get the format time for now
    if (time == nullptr) {
        now = std::time(nullptr);
        time = &now;
    }

    char buf[32] = {0};
    switch (mode) {
    case (common::NORMAL):
        std::strftime(buf, 32, "%Y-%m-%d-%H:%M:%S", std::localtime(time));
        break;
    case (common::SQLITE):
        std::strftime(buf, 32, "%Y_%m_%d_%H_%M_%S", std::localtime(time));
        break;
    case (common::MINSEC):
        std::strftime(buf, 32, "%M:%S", std::localtime(time));
        break;
    case (common::DAY_TIME):
        std::strftime(buf, 32, "%d-%H:%M:%S", std::localtime(time));
        break;
    default:
        common::panic(FILENAME, "formatTime");
        break;
    }

    return std::string(buf);
}

void panic(const std::string& filename, const std::string& funcname, const std::string& misc) {
    std::cerr << "<error> <" << filename << "> <" << funcname << "> " << misc
              << std::endl;
    exit(1);
}

}
// =============================================================================

size_t get_page::write_data(void* dataptr,     //pointer to data from curl
                                   size_t size,       //size of each data element
                                   size_t nmemb,      //num of data elements
                                   void* outstream) { //data output stream
    ((std::string*)outstream)->append((char*)dataptr, size * nmemb);
    return size * nmemb;
}

int get_page::get_page(const std::string& url, 
                       const std::vector<std::string>& headers, 
                       std::string& data) {
    CURL* curl = curl_easy_init();
    
    curl_global_init(CURL_GLOBAL_DEFAULT);

    //configure headers
    struct curl_slist* full_header = NULL;
    for (const std::string& header : headers) {
        full_header = curl_slist_append(full_header, header.data());
    }
    
    //get page
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url.data());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, full_header);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, get_page::write_data);
        curl_easy_setopt(curl, CURLOPT_ACCEPT_ENCODING, ""); //could make it faster

        // <DEBUG>
        // curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
        // curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1L);

        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &data);

        CURLcode res;
        if ((res = curl_easy_perform(curl)))
            while ( (res = curl_easy_perform(curl)) ) {
                std::cerr << "<error> <common.cpp> cURL connection error " << res << ", trying again in " 
                          << std::to_string(constant::CONNECTION_TIMEOUT_WAIT) << " seconds." << std::endl;
                std::this_thread::sleep_for(std::chrono::seconds(constant::CONNECTION_TIMEOUT_WAIT));
            }
    }
    else return 1;

    //clean
    curl_slist_free_all(full_header);
    curl_easy_cleanup(curl);
    curl_global_cleanup();
    return 0;
}