#include "core/BrowserController.h"
#include "Constants.h"
#include <iostream>

#ifdef _WIN32
    #include <windows.h>
#endif


BrowserController::BrowserController(std::shared_ptr<Logger> loggerInstance, bool enable)
    : logger(loggerInstance), browserTabOpened(false), enabled(enable) {
    
    std::cout << "[BrowserController] Initialized. Enabled: " << (enabled ? "YES" : "NO") << std::endl;
    logger->Info("BrowserController initialized. Enabled: " + std::string(enabled ? "YES" : "NO"), 
                "BrowserController");
}


void BrowserController::OpenBrowserWindows(const std::string& url) {
#ifdef _WIN32
    std::cout << "[BrowserController] Windows detected, using ShellExecuteA" << std::endl;
    logger->Info("Opening browser on Windows: " + url, "BrowserController");
    
    HINSTANCE result = ShellExecuteA(0, "open", url.c_str(), 0, 0, SW_SHOW);
    
    INT_PTR resultCode = (INT_PTR)result;
    std::cout << "[BrowserController] ShellExecuteA returned: " << resultCode << std::endl;
    
    if (resultCode <= 32) {
        std::string error = "Failed to open browser (error code: " + std::to_string(resultCode) + ")";
        std::cerr << "[BrowserController] ERROR: " << error << std::endl;
        logger->Error(error, "BrowserController");
        
        // Коды ошибок ShellExecute
        switch (resultCode) {
            case 0:  std::cerr << "  The system is out of memory" << std::endl; break;
            case 2:  std::cerr << "  File not found" << std::endl; break;
            case 3:  std::cerr << "  Path not found" << std::endl; break;
            case 5:  std::cerr << "  Access denied" << std::endl; break;
            case 8:  std::cerr << "  Out of memory" << std::endl; break;
            case 31: std::cerr << "  No association for file type" << std::endl; break;
            default: std::cerr << "  Unknown error" << std::endl; break;
        }
    } else {
        std::cout << "[BrowserController] Browser opened successfully!" << std::endl;
        logger->Success("Browser opened successfully", "BrowserController");
    }
#else
    std::cout << "[BrowserController] Not Windows, skipping" << std::endl;
#endif
}


void BrowserController::OpenBrowserMacOS(const std::string& url) {
#ifdef __APPLE__
    std::cout << "[BrowserController] macOS detected, using 'open' command" << std::endl;
    logger->Info("Opening browser on macOS: " + url, "BrowserController");
    
    std::string command = "open \"" + url + "\"";
    int result = system(command.c_str());
    
    std::cout << "[BrowserController] Command returned: " << result << std::endl;
    
    if (result != 0) {
        std::string error = "Failed to open browser (code: " + std::to_string(result) + ")";
        std::cerr << "[BrowserController] ERROR: " << error << std::endl;
        logger->Error(error, "BrowserController");
    } else {
        std::cout << "[BrowserController] Browser opened successfully!" << std::endl;
        logger->Success("Browser opened successfully", "BrowserController");
    }
#endif
}


void BrowserController::OpenBrowserLinux(const std::string& url) {
#if !defined(_WIN32) && !defined(__APPLE__)
    std::cout << "[BrowserController] Linux detected, using 'xdg-open' command" << std::endl;
    logger->Info("Opening browser on Linux: " + url, "BrowserController");
    
    std::string command = "xdg-open \"" + url + "\" 2>/dev/null &";
    int result = system(command.c_str());
    
    std::cout << "[BrowserController] Command returned: " << result << std::endl;
    
    if (result != 0) {
        std::string error = "Failed to open browser (code: " + std::to_string(result) + ")";
        std::cerr << "[BrowserController] ERROR: " << error << std::endl;
        logger->Error(error, "BrowserController");
    } else {
        std::cout << "[BrowserController] Browser opened successfully!" << std::endl;
        logger->Success("Browser opened successfully", "BrowserController");
    }
#endif
}


void BrowserController::OpenStream(const std::string& streamerName) {
    std::cout << "\n========================================" << std::endl;
    std::cout << "[BrowserController] OpenStream CALLED" << std::endl;
    std::cout << "  Streamer: " << streamerName << std::endl;
    std::cout << "  Enabled: " << (enabled ? "YES" : "NO") << std::endl;
    std::cout << "  Already opened: " << (browserTabOpened ? "YES" : "NO") << std::endl;
    std::cout << "========================================\n" << std::endl;
    
    logger->Info("OpenStream called for: " + streamerName, "BrowserController");
    
    if (!enabled) {
        std::cout << "[BrowserController] Browser opening is DISABLED in config" << std::endl;
        logger->Info("Browser opening disabled in config", "BrowserController");
        return;
    }
    
    if (browserTabOpened) {
        std::cout << "[BrowserController] Browser tab ALREADY OPENED, skipping" << std::endl;
        logger->Info("Browser already opened, skipping", "BrowserController");
        std::cout << "  (Browser already open)" << std::endl;
        return;
    }
    
    std::string streamUrl = std::string(Constants::TWITCH_BASE_URL) + streamerName;
    
    std::cout << "[BrowserController] Opening URL: " << streamUrl << std::endl;
    logger->Success("Opening stream in default browser: " + streamUrl, "BrowserController");
    std::cout << "  [BROWSER] Opening " << streamerName << " stream..." << std::endl;
    
#ifdef _WIN32
    std::cout << "[BrowserController] Platform: WINDOWS" << std::endl;
    OpenBrowserWindows(streamUrl);
#elif __APPLE__
    std::cout << "[BrowserController] Platform: macOS" << std::endl;
    OpenBrowserMacOS(streamUrl);
#else
    std::cout << "[BrowserController] Platform: LINUX" << std::endl;
    OpenBrowserLinux(streamUrl);
#endif
    
    browserTabOpened = true;
    std::cout << "[BrowserController] browserTabOpened set to TRUE" << std::endl;
    std::cout << "========================================\n" << std::endl;
}