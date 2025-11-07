#ifndef BROWSER_CONTROLLER_H
#define BROWSER_CONTROLLER_H

#include <string>
#include <memory>
#include "Logger.h"


class BrowserController {
private:
    std::shared_ptr<Logger> logger;
    bool browserTabOpened;
    bool enabled;
    
    void OpenBrowserWindows(const std::string& url);
    void OpenBrowserMacOS(const std::string& url);
    void OpenBrowserLinux(const std::string& url);

public:
    BrowserController(std::shared_ptr<Logger> loggerInstance, bool enable = true);
    
    // Просто открыть браузер (без задержек)
    void OpenStream(const std::string& streamerName);
    
    bool IsTabOpened() const { return browserTabOpened; }
    void SetEnabled(bool enable) { enabled = enable; }
    bool IsEnabled() const { return enabled; }
};

#endif // BROWSER_CONTROLLER_H