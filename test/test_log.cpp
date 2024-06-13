#include "../src/log/log.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <sys/stat.h>

bool fileExists(const std::string& filename) {
    struct stat buffer;
    return (stat(filename.c_str(), &buffer) == 0);
}

std::string readFileContents(const std::string& filename) {
    std::ifstream file(filename);
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

void testLogInitialization() {
    Log& log = Log::instance();
    log.init(DEBUG, "./testlog", ".log");
    std::cout << "Log system initialized.\n";
}

void testLogWriting() {
    Log& log = Log::instance();
    LOG_DEBUG("This is a debug message.");
    LOG_INFO("This is an info message.");
    LOG_WARN("This is a warning message.");
    LOG_ERROR("This is an error message.");
    std::cout << "Log messages written.\n";
}

void testLogFileCreation() {
    std::string filename = "./testlog/2024_06_13.log"; // adjust date accordingly
    if (fileExists(filename)) {
        std::cout << "Log file created: " << filename << "\n";
        std::string contents = readFileContents(filename);
        std::cout << "Log file contents:\n" << contents << "\n";
    }
    else {
        std::cerr << "Log file not created: " << filename << "\n";
    }
}

int main() {
    testLogInitialization();
    testLogWriting();
    testLogFileCreation();
    return 0;
}
