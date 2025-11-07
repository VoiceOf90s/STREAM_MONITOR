#include "StreamMonitor.h"
#include "MultiStreamMonitor.h"
#include "BrowserController.h"
#include "Logger.h"
#include "Constants.h"
#include "StringUtils.h"
#include <iostream>
#include <string>
#include <csignal>
#include <atomic>
#include <memory>


// Глобальные переменные для signal handler
std::atomic<bool> g_shutdownRequested(false);
std::unique_ptr<StreamMonitor> g_singleMonitor;
std::unique_ptr<MultiStreamMonitor> g_multiMonitor;


// Signal handler для graceful shutdown
void SignalHandler(int signal) {
    if (signal == SIGINT || signal == SIGTERM) {
        std::cout << "\n\n[SIGNAL] Shutdown signal received. Stopping gracefully..." << std::endl;
        g_shutdownRequested = true;
        
        if (g_singleMonitor) {
            g_singleMonitor->Stop();
        }
        if (g_multiMonitor) {
            g_multiMonitor->StopAll();
        }
    }
}


void SetupSignalHandlers() {
    std::signal(SIGINT, SignalHandler);
    std::signal(SIGTERM, SignalHandler);
}


void PrintWelcomeBanner() {
    std::cout << "\n";
    std::cout << "+=======================================+" << std::endl;
    std::cout << "|   Twitch Stream Monitor v2.3          |" << std::endl;
    std::cout << "|   Refactored | Thread-Safe | Stable  |" << std::endl;
    std::cout << "+=======================================+" << std::endl;
    std::cout << std::endl;
}


void PrintUsageInstructions() {
    std::cout << "Usage:" << std::endl;
    std::cout << "  Single streamer:" << std::endl;
    std::cout << "    stream_monitor <streamer_name> [config_file]" << std::endl;
    std::cout << std::endl;
    std::cout << "  Multiple streamers:" << std::endl;
    std::cout << "    stream_monitor --multi [streamers_file]" << std::endl;
    std::cout << std::endl;
    std::cout << "  Show statistics:" << std::endl;
    std::cout << "    stream_monitor --stats <streamer_name>" << std::endl;
    std::cout << std::endl;
    std::cout << "Examples:" << std::endl;
    std::cout << "    stream_monitor lydiaviolet" << std::endl;
    std::cout << "    stream_monitor shroud my_config.ini" << std::endl;
    std::cout << "    stream_monitor --multi streamers.txt" << std::endl;
    std::cout << "    stream_monitor --stats lydiaviolet" << std::endl;
    std::cout << std::endl;
}


void ShowStatistics(const std::string& streamerName) {
    if (!StringUtils::IsValidStreamerName(streamerName)) {
        std::cerr << "Error: Invalid streamer name '" << streamerName << "'" << std::endl;
        return;
    }
    
    try {
        std::cout << "\nLoading statistics for: " << streamerName << "\n" << std::endl;
        
        Statistics stats(streamerName, "stats/stats_" + streamerName + ".json");
        stats.PrintSummary();
        
    } catch (const std::exception& e) {
        std::cerr << "Error loading statistics: " << e.what() << std::endl;
    }
}


// ТЕСТОВАЯ ФУНКЦИЯ - добавить ДО main()
void TestBrowserOpening() {
    std::cout << "\n=== BROWSER OPENING TEST ===" << std::endl;
    std::cout << "Testing browser opening functionality..." << std::endl;
    
    // Создаем папки
    #ifdef _WIN32
        system("if not exist logs mkdir logs");
    #else
        system("mkdir -p logs");
    #endif
    
    auto logger = std::make_shared<Logger>("logs/test.log");
    auto browser = std::make_unique<BrowserController>(logger, true);
    
    std::cout << "\nAttempting to open browser for test..." << std::endl;
    browser->OpenStream("test_streamer");
    
    std::cout << "\n=== TEST COMPLETE ===" << std::endl;
    std::cout << "Did the browser open? (it should open https://www.twitch.tv/test_streamer)" << std::endl;
    std::cout << "Press Enter to continue..." << std::endl;
    std::cin.get();
}


int RunSingleMonitor(const std::string& streamerName, const std::string& configPath) {
    try {
        std::cout << "\n[RunSingleMonitor] ============================================" << std::endl;
        std::cout << "[RunSingleMonitor] Starting single monitor initialization..." << std::endl;
        std::cout << "[RunSingleMonitor] Streamer: " << streamerName << std::endl;
        std::cout << "[RunSingleMonitor] Config: " << configPath << std::endl;
        std::cout << "[RunSingleMonitor] ============================================\n" << std::endl;
        
        std::cout << "Initializing monitor for: " << streamerName << std::endl;
        std::cout << "Configuration: " << configPath << std::endl;
        std::cout << "Statistics will be saved to: stats/stats_" << streamerName << ".json" << std::endl;
        std::cout << "(Edit config.ini to customize settings)" << std::endl << std::endl;
        
        std::cout << "[RunSingleMonitor] Creating StreamMonitor object..." << std::endl;
        g_singleMonitor = std::make_unique<StreamMonitor>(streamerName, configPath);
        std::cout << "[RunSingleMonitor] StreamMonitor created successfully!" << std::endl;
        
        std::cout << "[RunSingleMonitor] Starting monitoring loop..." << std::endl;
        g_singleMonitor->StartMonitoring();
        
        std::cout << "[RunSingleMonitor] Monitoring loop exited (should not happen)" << std::endl;
        
        while (!g_shutdownRequested.load()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        
        std::cout << "\n[SHUTDOWN] Monitor stopped." << std::endl;
        return 0;
        
    } catch (const std::invalid_argument& e) {
        std::cerr << "\n[RunSingleMonitor] ERROR (invalid_argument): " << e.what() << std::endl;
        std::cerr << "Please provide a valid Twitch username.\n" << std::endl;
        return 1;
        
    } catch (const std::exception& e) {
        std::cerr << "\n[RunSingleMonitor] CRITICAL ERROR (exception): " << e.what() << std::endl;
        std::cerr << "Program terminated unexpectedly." << std::endl;
        std::cerr << "Check log file for details.\n" << std::endl;
        return 1;
    }
}


int RunMultiMonitor(const std::string& streamersFile) {
    try {
        std::cout << "Initializing multi-stream monitor..." << std::endl;
        std::cout << "Loading streamers from: " << streamersFile << std::endl << std::endl;
        
        g_multiMonitor = std::make_unique<MultiStreamMonitor>();
        
        if (!g_multiMonitor->LoadStreamersFromFile(streamersFile)) {
            std::cerr << "\nError: Failed to load streamers from " << streamersFile << std::endl;
            std::cerr << "Create " << streamersFile << " with one streamer name per line." << std::endl;
            std::cerr << "Example:" << std::endl;
            std::cerr << "  # My favorite streamers" << std::endl;
            std::cerr << "  shroud" << std::endl;
            std::cerr << "  pokimane" << std::endl;
            std::cerr << "  xqc" << std::endl << std::endl;
            return 1;
        }
        
        g_multiMonitor->PrintStatus();
        g_multiMonitor->StartAll();
        
        while (!g_shutdownRequested.load() && g_multiMonitor->IsRunning()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        
        if (!g_shutdownRequested.load()) {
            std::cout << "\nPress Enter to exit..." << std::endl;
            std::cin.get();
        }
        
        std::cout << "\n[SHUTDOWN] All monitors stopped." << std::endl;
        return 0;
        
    } catch (const std::exception& e) {
        std::cerr << "\nCRITICAL ERROR in multi-monitor mode: " << e.what() << std::endl;
        std::cerr << "Check log file for details.\n" << std::endl;
        return 1;
    }
}


int main(int argc, char* argv[]) {
    // Установка signal handlers
    SetupSignalHandlers();
    
    // Показываем текущую директорию
    std::cout << "Current directory: ";
    #ifdef _WIN32
        system("cd");
    #else
        system("pwd");
    #endif
    std::cout << std::endl;
    
    // Создаем папки
    #ifdef _WIN32
        system("if not exist logs mkdir logs");
        system("if not exist stats mkdir stats");
    #else
        system("mkdir -p logs");
        system("mkdir -p stats");
    #endif
    
    std::cout << "Folders created/checked" << std::endl << std::endl;
    
    // ТЕСТ: Раскомментируйте следующую строку для теста браузера
    // TestBrowserOpening();
    
    PrintWelcomeBanner();
    
    // Команда --stats
    if (argc > 2 && std::string(argv[1]) == "--stats") {
        ShowStatistics(argv[2]);
        return 0;
    }
    
    // Команда --multi
    if (argc > 1 && std::string(argv[1]) == "--multi") {
        std::string streamersFile = (argc > 2) ? argv[2] : Constants::STREAMERS_LIST_FILE;
        return RunMultiMonitor(streamersFile);
    }
    
    // Команда --help
    if (argc > 1 && (std::string(argv[1]) == "--help" || std::string(argv[1]) == "-h")) {
        PrintUsageInstructions();
        return 0;
    }
    
    // Обычный режим - один стример
    std::string streamerName;
    std::string configPath = Constants::DEFAULT_CONFIG_FILE;
    
    if (argc > 1) {
        streamerName = argv[1];
        
        if (!StringUtils::IsValidStreamerName(streamerName)) {
            std::cerr << "ERROR: Invalid streamer name '" << streamerName << "'" << std::endl;
            std::cerr << "Streamer name must be 1-25 characters, alphanumeric and underscore only." << std::endl << std::endl;
            PrintUsageInstructions();
            return 1;
        }
        
        std::cout << "Monitoring streamer: " << streamerName << std::endl;
        
        if (argc > 2) {
            configPath = argv[2];
            std::cout << "Using config: " << configPath << std::endl;
        }
        std::cout << std::endl;
        
    } else {
        std::cout << "Enter Twitch streamer username: ";
        std::getline(std::cin, streamerName);
        streamerName = StringUtils::Trim(streamerName);
        std::cout << std::endl;
        
        if (streamerName.empty()) {
            std::cerr << "ERROR: Streamer name cannot be empty!" << std::endl << std::endl;
            PrintUsageInstructions();
            return 1;
        }
        
        if (!StringUtils::IsValidStreamerName(streamerName)) {
            std::cerr << "ERROR: Invalid streamer name '" << streamerName << "'" << std::endl;
            std::cerr << "Streamer name must be 1-25 characters, alphanumeric and underscore only." << std::endl << std::endl;
            return 1;
        }
    }
    
    return RunSingleMonitor(streamerName, configPath);
}