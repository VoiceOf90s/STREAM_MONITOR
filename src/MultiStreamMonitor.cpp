#include "MultiStreamMonitor.h"
#include "StringUtils.h"
#include "Constants.h"
#include <iostream>
#include <fstream>
#include <algorithm>
#include <iomanip>


MultiStreamMonitor::MultiStreamMonitor(const std::string& configPath)
    : isRunning(false) {
    
    config = std::make_shared<Config>(configPath);
    config->Load();
    
    logger = std::make_shared<Logger>(
        config->GetString("log_file", Constants::DEFAULT_LOG_FILE),
        config->GetBool("verbose_logging", false)
    );
    
    logger->System("Multi-Stream Monitor initialized", "MultiStreamMonitor");
}


MultiStreamMonitor::~MultiStreamMonitor() {
    StopAll();
    logger->System("Multi-Stream Monitor destroyed", "MultiStreamMonitor");
}


void MultiStreamMonitor::MonitorThreadFunction(StreamMonitor* monitor) {
    try {
        logger->Info("Starting monitor thread for: " + monitor->GetStreamerName(), 
                    "MultiStreamMonitor");
        monitor->StartMonitoring();
    } catch (const std::exception& e) {
        logger->Error("Error in monitor thread: " + std::string(e.what()), 
                     "MultiStreamMonitor");
    }
}


bool MultiStreamMonitor::AddStreamer(const std::string& streamerName) {
    std::lock_guard<std::mutex> lock(monitorsMutex);
    
    // Валидация имени стримера
    if (!StringUtils::IsValidStreamerName(streamerName)) {
        logger->Error("Invalid streamer name: " + streamerName, "MultiStreamMonitor");
        std::cerr << "Error: Invalid streamer name '" << streamerName << "'" << std::endl;
        return false;
    }
    
    // Проверка на дубликаты
    for (const auto& info : monitors) {
        if (info.streamerName == streamerName) {
            logger->Warning("Streamer " + streamerName + " already being monitored", 
                           "MultiStreamMonitor");
            std::cerr << "Streamer " << streamerName << " is already being monitored" << std::endl;
            return false;
        }
    }
    
    try {
        MonitorInfo info;
        info.streamerName = streamerName;
        info.monitor = std::make_unique<StreamMonitor>(streamerName, "config.ini");
        info.isRunning = false;
        
        monitors.push_back(std::move(info));
        
        logger->Success("Added streamer: " + streamerName, "MultiStreamMonitor");
        std::cout << "Added streamer: " << streamerName << std::endl;
        return true;
        
    } catch (const std::exception& e) {
        logger->Error("Failed to add streamer " + streamerName + ": " + e.what(), 
                     "MultiStreamMonitor");
        std::cerr << "Error adding streamer: " << e.what() << std::endl;
        return false;
    }
}


bool MultiStreamMonitor::RemoveStreamer(const std::string& streamerName) {
    std::lock_guard<std::mutex> lock(monitorsMutex);
    
    auto it = std::find_if(monitors.begin(), monitors.end(),
        [&streamerName](const MonitorInfo& info) {
            return info.streamerName == streamerName;
        });
    
    if (it == monitors.end()) {
        logger->Warning("Streamer " + streamerName + " not found", "MultiStreamMonitor");
        std::cerr << "Streamer " << streamerName << " not found" << std::endl;
        return false;
    }
    
    // Graceful остановка потока
    if (it->isRunning && it->monitor) {
        logger->Info("Stopping monitor for: " + streamerName, "MultiStreamMonitor");
        it->monitor->Stop();
        
        if (it->thread && it->thread->joinable()) {
            logger->Debug("Waiting for thread to finish: " + streamerName, "MultiStreamMonitor");
            it->thread->join();
        }
    }
    
    monitors.erase(it);
    
    logger->Success("Removed streamer: " + streamerName, "MultiStreamMonitor");
    std::cout << "Removed streamer: " << streamerName << std::endl;
    return true;
}


std::vector<std::string> MultiStreamMonitor::GetStreamers() const {
    std::lock_guard<std::mutex> lock(monitorsMutex);
    
    std::vector<std::string> streamers;
    streamers.reserve(monitors.size());
    
    for (const auto& info : monitors) {
        streamers.push_back(info.streamerName);
    }
    
    return streamers;
}


void MultiStreamMonitor::StartAll() {
    std::lock_guard<std::mutex> lock(monitorsMutex);
    
    if (monitors.empty()) {
        logger->Warning("No streamers to monitor", "MultiStreamMonitor");
        std::cout << "No streamers to monitor. Add some first!" << std::endl;
        return;
    }
    
    logger->System("Starting monitoring for " + std::to_string(monitors.size()) + " streamer(s)", 
                   "MultiStreamMonitor");
    std::cout << "\nStarting monitoring for " << monitors.size() << " streamer(s)...\n" << std::endl;
    
    for (auto& info : monitors) {
        if (!info.isRunning) {
            try {
                info.thread = std::make_unique<std::thread>(
                    &MultiStreamMonitor::MonitorThreadFunction, 
                    this, 
                    info.monitor.get()
                );
                
                info.isRunning = true;
                
                logger->Success("Started monitoring: " + info.streamerName, "MultiStreamMonitor");
                std::cout << "  ✓ Started monitoring: " << info.streamerName << std::endl;
                
                // Задержка между запусками потоков
                std::this_thread::sleep_for(
                    std::chrono::milliseconds(Constants::THREAD_START_DELAY_MS)
                );
                
            } catch (const std::exception& e) {
                logger->Error("Failed to start thread for " + info.streamerName + ": " + e.what(), 
                             "MultiStreamMonitor");
                std::cerr << "  ✗ Failed to start: " << info.streamerName << std::endl;
            }
        }
    }
    
    isRunning = true;
    
    logger->System("All monitors started", "MultiStreamMonitor");
    std::cout << "\nAll monitors started! Press Ctrl+C to stop.\n" << std::endl;
}


void MultiStreamMonitor::StopAll() {
    std::lock_guard<std::mutex> lock(monitorsMutex);
    
    if (!isRunning.load()) {
        return;
    }
    
    logger->System("Stopping all monitors (graceful shutdown)", "MultiStreamMonitor");
    std::cout << "\nStopping all monitors (graceful shutdown)..." << std::endl;
    
    // Сначала отправляем сигнал остановки всем мониторам
    for (auto& info : monitors) {
        if (info.isRunning && info.monitor) {
            logger->Debug("Sending stop signal to: " + info.streamerName, "MultiStreamMonitor");
            info.monitor->Stop();
        }
    }
    
    // Теперь ждем завершения всех потоков
    for (auto& info : monitors) {
        if (info.thread && info.thread->joinable()) {
            logger->Debug("Waiting for thread to finish: " + info.streamerName, "MultiStreamMonitor");
            
            try {
                info.thread->join();
                info.isRunning = false;
                logger->Success("Stopped: " + info.streamerName, "MultiStreamMonitor");
                std::cout << "  ✓ Stopped: " << info.streamerName << std::endl;
                
            } catch (const std::exception& e) {
                logger->Error("Error joining thread for " + info.streamerName + ": " + e.what(), 
                             "MultiStreamMonitor");
                std::cerr << "  ✗ Error stopping: " << info.streamerName << std::endl;
            }
        }
    }
    
    isRunning = false;
    
    logger->System("All monitors stopped", "MultiStreamMonitor");
    std::cout << "\nAll monitors stopped gracefully." << std::endl;
}


void MultiStreamMonitor::PrintStatus() const {
    std::lock_guard<std::mutex> lock(monitorsMutex);
    
    std::cout << "\n╔════════════════════════════════════════════════╗" << std::endl;
    std::cout << "║        MULTI-STREAM MONITOR STATUS            ║" << std::endl;
    std::cout << "╠════════════════════════════════════════════════╣" << std::endl;
    std::cout << "║ Total streamers: " << std::left << std::setw(29) 
              << monitors.size() << "║" << std::endl;
    std::cout << "║ System running:  " << std::left << std::setw(29) 
              << (isRunning.load() ? "YES" : "NO") << "║" << std::endl;
    std::cout << "╠════════════════════════════════════════════════╣" << std::endl;
    
    if (monitors.empty()) {
        std::cout << "║ No streamers configured                        ║" << std::endl;
    } else {
        for (const auto& info : monitors) {
            std::cout << "║ • " << std::left << std::setw(30) << info.streamerName;
            std::cout << std::right << std::setw(15) 
                     << (info.isRunning ? "[RUNNING]" : "[STOPPED]") 
                     << " ║" << std::endl;
        }
    }
    
    std::cout << "╚════════════════════════════════════════════════╝" << std::endl;
}


bool MultiStreamMonitor::LoadStreamersFromFile(const std::string& filePath) {
    logger->Info("Loading streamers from file: " + filePath, "MultiStreamMonitor");
    
    std::ifstream file(filePath);
    
    if (!file.is_open()) {
        logger->Error("Cannot open streamers file: " + filePath, "MultiStreamMonitor");
        std::cerr << "Cannot open streamers file: " << filePath << std::endl;
        return false;
    }
    
    std::string line;
    int lineNumber = 0;
    int loadedCount = 0;
    
    while (std::getline(file, line)) {
        lineNumber++;
        
        // Убираем пробелы
        line = StringUtils::Trim(line);
        
        // Пропускаем комментарии и пустые строки
        if (line.empty() || line[0] == '#') {
            continue;
        }
        
        // Добавляем стримера
        if (AddStreamer(line)) {
            loadedCount++;
        } else {
            logger->Warning("Failed to add streamer from line " + std::to_string(lineNumber) + 
                           ": " + line, "MultiStreamMonitor");
        }
    }
    
    file.close();
    
    logger->Success("Loaded " + std::to_string(loadedCount) + " streamer(s) from " + filePath, 
                   "MultiStreamMonitor");
    std::cout << "\nLoaded " << loadedCount << " streamer(s) from " << filePath << std::endl;
    
    return loadedCount > 0;
}