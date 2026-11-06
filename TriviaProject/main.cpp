#include "Server.h"
#include <iostream>
#include <string>
#include <stdexcept>

int main(int argc, char* argv[]) {
    int port = 8080;

    // Command-line argument parsing for server port override
    if (argc >= 2) {
        try {
            port = std::stoi(argv[1]);

            // Validate port range to avoid reserved/system ports
            if (port < 1024 || port > 65535) {
                std::cerr << "[Error] Port must be between 1024 and 65535." << std::endl;
                return 1;
            }
        }
        catch (...) {
            // Handles non-numeric or invalid string input for port
            std::cerr << "[Error] Invalid port argument." << std::endl;
            return 1;
        }
    }

    // Get singleton instance with port, then run
    Server::getInstance(port).run();

    return 0;
}