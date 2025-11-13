#include "StreamMonitor.h"
#include "StringUtils.h"
#include "Constants.h"
#include <iostream>
#include <thread>
#include <chrono>


StreamMonitor::StreamMonitor(const std::string& streamer, const std::string& configPath)
    : streamerName(streamer), wasOnlineBefore(false), shouldStop(false), 
      lastOnlineTimestamp(0) {
    
    std::cout << "[DEBUG] StreamMonitor constructor START" << std::endl;
    
    if (!StringUtils::IsValidStreamerName(streamerName)) {
        throw std::invalid_argument("Invalid streamer name: " + streamerName);
    }
    std::cout << "[DEBUG] Streamer name validated: " << streamerName << std::endl;
    
    // Создаем папки
    #ifdef _WIN32
        system("if not exist logs mkdir logs >nul 2>&1");
        system("if not exist stats mkdir stats >nul 2>&1");
    #else
        system("mkdir -p logs 2>/dev/null");
        system("mkdir -p stats 2>/dev/null");
    #endif
    std::cout << "[DEBUG] Folders created" << std::endl;
    
    std::cout << "[DEBUG] Creating logger..." << std::endl;
    logger = std::make_shared<Logger>("logs/stream_monitor.log");
    std::cout << "[DEBUG] Logger created" << std::endl;
    
    std::cout << "[DEBUG] Creating config..." << std::endl;
    config = std::make_unique<Config>(configPath);
    std::cout << "[DEBUG] Config created" << std::endl;
    
    std::cout << "[DEBUG] Loading config..." << std::endl;
    if (!config->Load()) {
        logger->Warning("Failed to load config, using defaults", "StreamMonitor");
        std::cout << "[DEBUG] Config load failed, using defaults" << std::endl;
    } else {
        std::cout << "[DEBUG] Config loaded successfully" << std::endl;
    }
    
    logger->SetVerbose(config->GetBool("verbose_logging", false));
    std::cout << "[DEBUG] Verbose logging set" << std::endl;
    
    checkInterval = config->GetInt("check_interval", Constants::DEFAULT_CHECK_INTERVAL);
    checkIntervalFast = config->GetInt("check_interval_fast", Constants::FAST_CHECK_INTERVAL);
    fastModeDuration = config->GetInt("fast_mode_duration", Constants::FAST_MODE_DURATION);
    enableNotifications = config->GetBool("enable_notifications", true);
    enableStatistics = config->GetBool("enable_statistics", true);
    std::cout << "[DEBUG] Config values loaded" << std::endl;
    
    std::cout << "[DEBUG] Creating notification..." << std::endl;
    notification = std::make_unique<Notification>(enableNotifications);
    std::cout << "[DEBUG] Notification created" << std::endl;
    
    std::cout << "[DEBUG] Creating statistics..." << std::endl;
    statistics = std::make_unique<Statistics>(streamerName, "stats/stats_" + streamerName + ".json");
    std::cout << "[DEBUG] Statistics created" << std::endl;
    
    std::cout << "[DEBUG] Creating webScraper..." << std::endl;
    std::cout << "[DEBUG] This might take a moment (initializing cURL)..." << std::endl;
    webScraper = std::make_unique<WebScraper>(logger, *config);
    std::cout << "[DEBUG] WebScraper created" << std::endl;
    
    bool openBrowser = config->GetBool("open_browser", true);
    std::cout << "[DEBUG] Creating browserController (enabled: " << (openBrowser ? "YES" : "NO") << ")..." << std::endl;
    browserController = std::make_unique<BrowserController>(logger, openBrowser);
    std::cout << "[DEBUG] BrowserController created" << std::endl;
    
    logger->System("=== Stream Monitor v2.3 initialized ===", "StreamMonitor");
    logger->System("Monitoring streamer: " + streamerName, "StreamMonitor");
    logger->Info("Configuration loaded from: " + configPath, "StreamMonitor");
    logger->Info("Features: Notifications=" + std::string(enableNotifications ? "ON" : "OFF") +
                ", Statistics=" + std::string(enableStatistics ? "ON" : "OFF"), "StreamMonitor");
    
    std::cout << "[DEBUG] Checking if WebScraper initialized..." << std::endl;
    if (!webScraper->IsInitialized()) {
        std::cout << "[DEBUG] ERROR: WebScraper NOT initialized!" << std::endl;
        logger->Critical("Failed to initialize WebScraper", "StreamMonitor");
        throw std::runtime_error("WebScraper initialization failed");
    }
    std::cout << "[DEBUG] WebScraper is initialized" << std::endl;
    
    std::cout << "[DEBUG] StreamMonitor constructor END" << std::endl;
    std::cout << "[DEBUG] ============================================\n" << std::endl;
}

StreamMonitor::~StreamMonitor() {
    logger->System("=== Stream Monitor shutdown ===", "StreamMonitor");
}


long long StreamMonitor::GetUnixTimestamp() const {
    return std::chrono::duration_cast<std::chrono::seconds>(
        std::chrono::system_clock::now().time_since_epoch()
    ).count();
}


int StreamMonitor::GetCurrentCheckInterval() {
    long long currentTime = GetUnixTimestamp();
    long long timeSinceLastOnline = currentTime - lastOnlineTimestamp;
    
    if (lastOnlineTimestamp > 0 && timeSinceLastOnline < fastModeDuration) {
        logger->Debug("Using FAST check interval (" + std::to_string(checkIntervalFast) + "s)", 
                     "StreamMonitor");
        return checkIntervalFast;
    }
    
    return checkInterval;
}


void StreamMonitor::HandleStreamOnline() {
    std::cout << "\n!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!" << std::endl;
    std::cout << "!!! STREAM WENT ONLINE !!!" << std::endl;
    std::cout << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n" << std::endl;
    
    std::cout << "[ONLINE] " << streamerName << " started streaming!" << std::endl;
    logger->Event("Stream status changed: OFFLINE -> ONLINE", "StreamMonitor");
    
    std::cout << "[StreamMonitor] Sending notification..." << std::endl;
    if (enableNotifications && notification) {
        notification->NotifyStreamOnline(streamerName);
    }
    
    std::cout << "[StreamMonitor] Recording statistics..." << std::endl;
    if (enableStatistics && statistics) {
        statistics->RecordStreamOnline();
    }
    
    std::cout << "[StreamMonitor] Opening browser..." << std::endl;
    std::cout << "  browserController pointer: " << (browserController ? "VALID" : "NULL") << std::endl;
    
    if (browserController) {
        std::cout << "  Calling browserController->OpenStream(\"" << streamerName << "\")..." << std::endl;
        browserController->OpenStream(streamerName);
        std::cout << "  browserController->OpenStream() returned" << std::endl;
    } else {
        std::cout << "  ERROR: browserController is NULL!" << std::endl;
    }
    
    wasOnlineBefore = true;
    lastOnlineTimestamp = GetUnixTimestamp();
    
    std::cout << "\n[StreamMonitor] HandleStreamOnline completed\n" << std::endl;
}


void StreamMonitor::HandleStreamOffline() {
    std::cout << "[OFFLINE] " << streamerName << " ended stream" << std::endl;
    logger->Event("Stream status changed: ONLINE -> OFFLINE", "StreamMonitor");
    
    if (enableNotifications && notification) {
        notification->NotifyStreamOffline(streamerName);
    }
    
    if (enableStatistics && statistics) {
        statistics->RecordStreamOffline();
    }
    
    wasOnlineBefore = false;
    lastOnlineTimestamp = GetUnixTimestamp();
}


void StreamMonitor::StartMonitoring() {
    logger->System("Starting monitoring loop (v2.3)", "StreamMonitor");
    
    std::cout << "\n+=======================================+" << std::endl;
    std::cout << "|   Twitch Stream Monitor v2.3          |" << std::endl;
    std::cout << "|   Refactored | Thread-Safe | Stable  |" << std::endl;
    std::cout << "+=======================================+" << std::endl;
    std::cout << std::endl;
    
    std::cout << "Monitoring: " << streamerName << std::endl;
    std::cout << "Features:" << std::endl;
    std::cout << "  - Notifications: " << (enableNotifications ? "ON" : "OFF") << std::endl;
    std::cout << "  - Statistics: " << (enableStatistics ? "ON" : "OFF") << std::endl;
    std::cout << "  - Check intervals: " << checkInterval << "s / " << checkIntervalFast << "s" << std::endl;
    std::cout << "  - Browser control: " << (browserController->IsEnabled() ? "ON" : "OFF") << std::endl;
    std::cout << std::endl;
    std::cout << "Press Ctrl+C to stop (graceful shutdown supported)" << std::endl;
    std::cout << std::endl;
    
    int checkCount = 0;
    
    while (!shouldStop.load()) {
        try {
            checkCount++;
            std::cout << "\n========== CHECK #" << checkCount << " ==========" << std::endl;
            
            auto checkStartTime = std::chrono::steady_clock::now();
            
            std::cout << "[DEBUG] Calling webScraper->CheckStreamStatus(\"" << streamerName << "\")..." << std::endl;
            bool isCurrentlyOnline = webScraper->CheckStreamStatus(streamerName);
            
            auto checkEndTime = std::chrono::steady_clock::now();
            auto checkDuration = std::chrono::duration_cast<std::chrono::milliseconds>(
                checkEndTime - checkStartTime
            ).count();
            
            std::cout << "[DEBUG] CheckStreamStatus returned: " << (isCurrentlyOnline ? "TRUE (ONLINE)" : "FALSE (OFFLINE)") << std::endl;
            std::cout << "[DEBUG] Check duration: " << checkDuration << "ms" << std::endl;
            
            if (enableStatistics && statistics) {
                statistics->RecordCheck(checkDuration);
            }
            
            bool wasPreviouslyOnline = wasOnlineBefore.load();
            std::cout << "[DEBUG] wasOnlineBefore: " << (wasPreviouslyOnline ? "TRUE" : "FALSE") << std::endl;
            std::cout << "[DEBUG] isCurrentlyOnline: " << (isCurrentlyOnline ? "TRUE" : "FALSE") << std::endl;
            
            // КРИТИЧЕСКАЯ ПРОВЕРКА
            if (isCurrentlyOnline && !wasPreviouslyOnline) {
                std::cout << "\n[DEBUG] *** CONDITION MET: Stream went from OFFLINE to ONLINE ***" << std::endl;
                std::cout << "[DEBUG] *** Calling HandleStreamOnline() ***\n" << std::endl;
                HandleStreamOnline();
            } 
            else if (!isCurrentlyOnline && wasPreviouslyOnline) {
                std::cout << "\n[DEBUG] *** CONDITION MET: Stream went from ONLINE to OFFLINE ***" << std::endl;
                std::cout << "[DEBUG] *** Calling HandleStreamOffline() ***\n" << std::endl;
                HandleStreamOffline();
            }
            else if (isCurrentlyOnline) {
                std::cout << "[OK] " << streamerName << " still online (" << checkDuration << "ms)" << std::endl;
                lastOnlineTimestamp = GetUnixTimestamp();
            }
            else {
                int interval = GetCurrentCheckInterval();
                std::cout << "[WAIT] " << streamerName << " still offline (" 
                         << checkDuration << "ms, next in " << interval << "s)" << std::endl;
            }
            
        } catch (const std::exception& e) {
            std::string errorMsg = "Exception in monitoring loop: " + std::string(e.what());
            logger->Critical(errorMsg, "StreamMonitor");
            std::cerr << "[ERROR] " << streamerName << ": " << errorMsg << std::endl;
        }
        
        int sleepInterval = GetCurrentCheckInterval();
        std::cout << "[DEBUG] Sleeping for " << sleepInterval << " seconds..." << std::endl;
        
        for (int i = 0; i < sleepInterval && !shouldStop.load(); ++i) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    }
    
    logger->System("Monitoring loop stopped gracefully", "StreamMonitor");
    std::cout << "\n[STOPPED] Monitoring for " << streamerName << " stopped." << std::endl;
}


void StreamMonitor::Stop() {
    logger->System("Stop requested", "StreamMonitor");
    shouldStop = true;
}


void StreamMonitor::EnableNotifications(bool enable) {
    enableNotifications = enable;
    if (notification) {
        notification->SetEnabled(enable);
    }
    logger->Info("Notifications " + std::string(enable ? "enabled" : "disabled"), "StreamMonitor");
}


void StreamMonitor::EnableStatistics(bool enable) {
    enableStatistics = enable;
    logger->Info("Statistics " + std::string(enable ? "enabled" : "disabled"), "StreamMonitor");
}


void StreamMonitor::ShowStatistics() const {
    if (statistics) {
        statistics->PrintSummary();
    } else {
        std::cout << "Statistics not available" << std::endl;
    }
}