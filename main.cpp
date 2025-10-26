#include "StreamMonitor.h"
#include <iostream>


void PrintWelcomeBanner() {
    std::cout << "\n";
    std::cout << "+=======================================+" << std::endl;
    std::cout << "|   Twitch Stream Monitor v2.1          |" << std::endl;
    std::cout << "|   Config | HTTP/2 | DNS Cache         |" << std::endl;
    std::cout << "+=======================================+" << std::endl;
    std::cout << std::endl;
}


void PrintUsageInstructions() {
    std::cout << "Usage:" << std::endl;
    std::cout << "   stream_monitor.exe <streamer_name> [config_file]" << std::endl;
    std::cout << std::endl;
    std::cout << "Examples:" << std::endl;
    std::cout << "   stream_monitor.exe lydiaviolet" << std::endl;
    std::cout << "   stream_monitor.exe shroud my_config.ini" << std::endl;
    std::cout << std::endl;
    std::cout << "Configuration:" << std::endl;
    std::cout << "   On first run, config.ini will be created automatically." << std::endl;
    std::cout << "   Edit config.ini to customize settings." << std::endl;
    std::cout << std::endl;
}


int main(int argc, char* argv[]) {
    PrintWelcomeBanner();
    
    std::string streamerName;
    std::string configPath = "config.ini";
    
    // Обработка аргументов командной строки
    if (argc > 1) {
        streamerName = argv[1];
        std::cout << "Monitoring streamer: " << streamerName << std::endl;
        
        if (argc > 2) {
            configPath = argv[2];
            std::cout << "Using config: " << configPath << std::endl;
        }
        std::cout << std::endl;
    } else {
        std::cout << "Enter Twitch streamer username: ";
        std::cin >> streamerName;
        std::cout << std::endl;
        
        if (streamerName.empty()) {
            std::cerr << "ERROR: Streamer name cannot be empty!" << std::endl << std::endl;
            PrintUsageInstructions();
            return 1;
        }
    }
    
    std::cout << "Configuration: " << configPath << std::endl;
    std::cout << "(Edit this file to customize settings)" << std::endl << std::endl;
    
    try {
        StreamMonitor monitor(streamerName, configPath);
        monitor.StartMonitoring();
    } 
    catch (const std::exception& e) {
        std::cerr << "\nCRITICAL ERROR: " << e.what() << std::endl;
        std::cerr << "Program terminated unexpectedly." << std::endl;
        std::cerr << "Check log file for details.\n" << std::endl;
        return 1;
    }
    
    return 0;
}