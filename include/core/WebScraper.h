#ifndef WEB_SCRAPER_H
#define WEB_SCRAPER_H

#include <string>
#include <memory>
#include <curl/curl.h>
#include "Logger.h"
#include "Config.h"
#include "HumanBehavior.h"


// RAII wrapper для CURL handle
class CurlHandle {
private:
    CURL* handle;

public:
    CurlHandle();
    ~CurlHandle();
    
    CurlHandle(const CurlHandle&) = delete;
    CurlHandle& operator=(const CurlHandle&) = delete;
    
    CurlHandle(CurlHandle&& other) noexcept;
    CurlHandle& operator=(CurlHandle&& other) noexcept;
    
    CURL* Get() { return handle; }
    bool IsValid() const { return handle != nullptr; }
};


struct CallbackData {
    std::string* buffer;
    bool* foundMarker;
    size_t maxSize;
};


// Класс для веб-скрапинга Twitch (БЕЗ API)
class WebScraper {
private:
    std::shared_ptr<Logger> logger;
    CurlHandle curlHandle;
    std::unique_ptr<HumanBehavior> humanBehavior;
    
    size_t maxHtmlSize;
    int timeout;
    int connectTimeout;
    bool useHttp2;
    int dnsCacheTimeout;
    bool sslVerifyPeer;
    bool sslVerifyHost;
    
    int requestCounter;
    
    static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp);
    
    void ConfigureCurlWithHumanHeaders();
    std::string DownloadPageHtml(const std::string& streamerName);
    bool ParseStreamStatus(const std::string& html);
    void MaybePerformExtraRequest(const std::string& streamerName);

public:
    WebScraper(std::shared_ptr<Logger> loggerInstance, const Config& config);
    ~WebScraper();
    
    // Основной метод: проверка статуса через скрапинг
    bool CheckStreamStatus(const std::string& streamerName);
    bool IsInitialized() const { return curlHandle.IsValid(); }
};

#endif // WEB_SCRAPER_H