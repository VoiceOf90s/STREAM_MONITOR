#include "StreamMonitor.h"
#include "MultiStreamMonitor.h"
#include <iostream>
#include <string>


void PrintWelcomeBanner() {
    std::cout << "\n";
    std::cout << "+=======================================+" << std::endl;
    std::cout << "|   Twitch Stream Monitor v2.2          |" << std::endl;
    std::cout << "|   Multi | Notifications | Statistics |" << std::endl;
    std::cout << "+=======================================+" << std::endl;
    std::cout << std::endl;
}


void PrintUsageInstructions() {
    std::cout << "Usage:" << std::endl;
    std::cout << "  Single streamer:" << std::endl;
    std::cout << "    stream_monitor.exe <streamer_name> [config_file]" << std::endl;
    std::cout << std::endl;
    std::cout << "  Multiple streamers:" << std::endl;
    std::cout << "    stream_monitor.exe --multi [streamers_file]" << std::endl;
    std::cout << std::endl;
    std::cout << "  Show statistics:" << std::endl;
    std::cout << "    stream_monitor.exe --stats <streamer_name>" << std::endl;
    std::cout << std::endl;
    std::cout << "Examples:" << std::endl;
    std::cout << "  stream_monitor.exe lydiaviolet" << std::endl;
    std::cout << "  stream_monitor.exe shroud my_config.ini" << std::endl;
    std::cout << "  stream_monitor.exe --multi streamers.txt" << std::endl;
    std::cout << "  stream_monitor.exe --stats lydiaviolet" << std::endl;
    std::cout << std::endl;
}


void ShowStatistics(const std::string& streamerName) {
    try {
        Statistics stats(streamerName, "stats_" + streamerName + ".json");
        stats.PrintSummary();
    } catch (const std::exception& e) {
        std::cerr << "Error loading statistics: " << e.what() << std::endl;
    }
}


int main(int argc, char* argv[]) {
    PrintWelcomeBanner();
    
    // Обработка команды --stats
    if (argc > 2 && std::string(argv[1]) == "--stats") {
        ShowStatistics(argv[2]);
        return 0;
    }
    
    // Обработка команды --multi
    if (argc > 1 && std::string(argv[1]) == "--multi") {
        std::string streamersFile = (argc > 2) ? argv[2] : "streamers.txt";
        
        try {
            MultiStreamMonitor multiMonitor;
            
            if (multiMonitor.LoadStreamersFromFile(streamersFile)) {
                multiMonitor.PrintStatus();
                multiMonitor.StartAll();
                
                // Главный поток ждет (в production нужен signal handler)
                std::cout << "\nPress Enter to stop monitoring..." << std::endl;
                std::cin.get();
                
                multiMonitor.StopAll();
            } else {
                std::cerr << "Failed to load streamers from " << streamersFile << std::endl;
                std::cerr << "Create " << streamersFile << " with one streamer name per line." << std::endl;
                return 1;
            }
        } catch (const std::exception& e) {
            std::cerr << "Error in multi-monitor mode: " << e.what() << std::endl;
            return 1;
        }
        
        return 0;
    }
    
    // Обычный режим - один стример
    std::string streamerName;
    std::string configPath = "config.ini";
    
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
    std::cout << "Statistics will be saved to: stats_" << streamerName << ".json" << std::endl;
    std::cout << "(Edit config.ini to customize settings)" << std::endl << std::endl;
    
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