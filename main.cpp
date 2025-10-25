#include "StreamMonitor.h"
#include <iostream>


void PrintWelcomeBanner() {
    std::cout << "\n╔═══════════════════════════════════════╗\n";
    std::cout << "║   Twitch Stream Monitor v2.0          ║\n";
    std::cout << "║   Simple Version - No API Required!   ║\n";
    std::cout << "╚═══════════════════════════════════════╝\n" << std::endl;
}


void PrintUsageInstructions() {
    std::cout << "📖 Usage:\n";
    std::cout << "   ./stream_monitor <streamer_name>\n";
    std::cout << "   Example: ./stream_monitor lydiaviolet\n\n";
    std::cout << "   Or run without arguments to enter streamer name interactively.\n" << std::endl;
}


int main(int argc, char* argv[]) {
    PrintWelcomeBanner();
    
    std::string streamerName;
    
    // Получение имени стримера из аргументов командной строки или через ввод
    if (argc > 1) {
        streamerName = argv[1];
        std::cout << "📌 Monitoring streamer: " << streamerName << "\n" << std::endl;
    } else {
        std::cout << "Enter Twitch streamer username: ";
        std::cin >> streamerName;
        std::cout << std::endl;
        
        if (streamerName.empty()) {
            std::cerr << "❌ ERROR: Streamer name cannot be empty!\n" << std::endl;
            PrintUsageInstructions();
            return 1;
        }
    }
    
    std::cout << "✅ No configuration needed - using web scraping!\n" << std::endl;
    
    // Создание и запуск монитора
    try {
        StreamMonitor monitor(streamerName);
        monitor.StartMonitoring();
    } 
    catch (const std::exception& e) {
        std::cerr << "\n❌ CRITICAL ERROR: " << e.what() << "\n"
                  << "Program terminated unexpectedly.\n"
                  << "Check log file '" << LOG_FILE_PATH << "' for details.\n" << std::endl;
        return 1;
    }
    
    return 0;
}