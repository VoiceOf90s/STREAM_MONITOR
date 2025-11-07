#include "WebScraper.h"
#include "Constants.h"
#include <iostream>
#include <algorithm>


// ==================== CurlHandle Implementation ====================

CurlHandle::CurlHandle() : handle(curl_easy_init()) {
    if (!handle) {
        std::cerr << "CRITICAL: Failed to initialize cURL handle" << std::endl;
    }
}


CurlHandle::~CurlHandle() {
    if (handle) {
        curl_easy_cleanup(handle);
        handle = nullptr;
    }
}


CurlHandle::CurlHandle(CurlHandle&& other) noexcept : handle(other.handle) {
    other.handle = nullptr;
}


CurlHandle& CurlHandle::operator=(CurlHandle&& other) noexcept {
    if (this != &other) {
        if (handle) {
            curl_easy_cleanup(handle);
        }
        handle = other.handle;
        other.handle = nullptr;
    }
    return *this;
}


// ==================== WebScraper Implementation ====================

WebScraper::WebScraper(std::shared_ptr<Logger> loggerInstance, const Config& config)
    : logger(loggerInstance), requestCounter(0) {
    
    std::cout << "[WebScraper] Constructor START" << std::endl;
    
    maxHtmlSize = static_cast<size_t>(config.GetInt("max_html_size", Constants::MAX_HTML_SIZE));
    timeout = config.GetInt("timeout", Constants::DEFAULT_TIMEOUT);
    connectTimeout = config.GetInt("connect_timeout", Constants::DEFAULT_CONNECT_TIMEOUT);
    useHttp2 = config.GetBool("use_http2", true);
    dnsCacheTimeout = config.GetInt("dns_cache_timeout", Constants::DNS_CACHE_TIMEOUT_SECONDS);
    sslVerifyPeer = config.GetBool("ssl_verify_peer", false);
    sslVerifyHost = config.GetBool("ssl_verify_host", false);
    
    std::cout << "[WebScraper] Config loaded" << std::endl;
    std::cout << "[WebScraper]   maxHtmlSize: " << maxHtmlSize << std::endl;
    std::cout << "[WebScraper]   timeout: " << timeout << std::endl;
    std::cout << "[WebScraper]   useHttp2: " << (useHttp2 ? "YES" : "NO") << std::endl;
    
    std::cout << "[WebScraper] Creating HumanBehavior..." << std::endl;
    humanBehavior = std::make_unique<HumanBehavior>();
    std::cout << "[WebScraper] HumanBehavior created" << std::endl;
    
    std::cout << "[WebScraper] Checking cURL handle..." << std::endl;
    if (!curlHandle.IsValid()) {
        std::cout << "[WebScraper] ERROR: cURL handle is INVALID!" << std::endl;
        logger->Critical("Failed to initialize cURL handle", "WebScraper");
        return;
    }
    std::cout << "[WebScraper] cURL handle is valid" << std::endl;
    
    std::cout << "[WebScraper] Configuring cURL with human headers..." << std::endl;
    ConfigureCurlWithHumanHeaders();
    std::cout << "[WebScraper] cURL configured" << std::endl;
    
    logger->Success("WebScraper initialized (NO API, only HTML parsing)", "WebScraper");
    std::cout << "[WebScraper] Constructor END\n" << std::endl;
}


WebScraper::~WebScraper() {
    logger->Debug("WebScraper destroyed", "WebScraper");
}


void WebScraper::ConfigureCurlWithHumanHeaders() {
    CURL* handle = curlHandle.Get();
    if (!handle) return;
    
    std::string userAgent = humanBehavior->GetSessionUserAgent();
    curl_easy_setopt(handle, CURLOPT_USERAGENT, userAgent.c_str());
    
    logger->Debug("Using User-Agent: " + userAgent, "WebScraper");
    
    curl_easy_setopt(handle, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(handle, CURLOPT_MAXREDIRS, 5L);
    curl_easy_setopt(handle, CURLOPT_SSL_VERIFYPEER, sslVerifyPeer ? 1L : 0L);
    curl_easy_setopt(handle, CURLOPT_SSL_VERIFYHOST, sslVerifyHost ? 2L : 0L);
    curl_easy_setopt(handle, CURLOPT_TIMEOUT, static_cast<long>(timeout));
    curl_easy_setopt(handle, CURLOPT_CONNECTTIMEOUT, static_cast<long>(connectTimeout));
    curl_easy_setopt(handle, CURLOPT_TCP_KEEPALIVE, 1L);
    curl_easy_setopt(handle, CURLOPT_TCP_NODELAY, 1L);
    curl_easy_setopt(handle, CURLOPT_ACCEPT_ENCODING, "gzip, deflate, br");
    
    if (useHttp2) {
        curl_easy_setopt(handle, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_2_0);
        logger->Debug("HTTP/2 enabled", "WebScraper");
    }
    
    if (dnsCacheTimeout > 0) {
        curl_easy_setopt(handle, CURLOPT_DNS_CACHE_TIMEOUT, static_cast<long>(dnsCacheTimeout));
    }
    
    curl_easy_setopt(handle, CURLOPT_COOKIEFILE, "");
}


size_t WebScraper::WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    size_t totalSize = size * nmemb;
    CallbackData* data = static_cast<CallbackData*>(userp);
    
    if (*(data->foundMarker)) {
        return totalSize;
    }
    
    if (data->buffer->length() >= data->maxSize) {
        return totalSize;
    }
    
    size_t remainingSpace = data->maxSize - data->buffer->length();
    size_t sizeToAdd = std::min(totalSize, remainingSpace);
    
    data->buffer->append(static_cast<char*>(contents), sizeToAdd);
    
    if (data->buffer->find(Constants::TwitchMarkers::IS_LIVE_BROADCAST) != std::string::npos ||
        data->buffer->find(Constants::TwitchMarkers::TYPE_LIVE) != std::string::npos ||
        data->buffer->find(Constants::TwitchMarkers::BROADCAST_TYPE) != std::string::npos) {
        *(data->foundMarker) = true;
    }
    
    return totalSize;
}


void WebScraper::MaybePerformExtraRequest(const std::string& streamerName) {
    if (humanBehavior->ShouldMakeExtraRequest() && requestCounter % 5 == 0) {
        logger->Debug("Performing extra request to main page (anti-detection)", "WebScraper");
        
        CURL* handle = curlHandle.Get();
        std::string mainPage = "https://www.twitch.tv/";
        
        curl_easy_setopt(handle, CURLOPT_URL, mainPage.c_str());
        curl_easy_setopt(handle, CURLOPT_NOBODY, 1L);
        
        humanBehavior->RandomDelay(300, 800);
        curl_easy_perform(handle);
        curl_easy_setopt(handle, CURLOPT_NOBODY, 0L);
    }
}


std::string WebScraper::DownloadPageHtml(const std::string& streamerName) {
    if (!curlHandle.IsValid()) {
        logger->Error("cURL handle not initialized", "WebScraper");
        return "";
    }
    
    humanBehavior->SimulateThinking();
    
    requestCounter++;
    MaybePerformExtraRequest(streamerName);
    
    std::string readBuffer;
    bool foundMarker = false;
    
    CallbackData callbackData = {&readBuffer, &foundMarker, maxHtmlSize};
    
    std::string url = std::string(Constants::TWITCH_BASE_URL) + streamerName;
    CURL* handle = curlHandle.Get();
    
    struct curl_slist* headers = nullptr;
    auto humanHeaders = humanBehavior->GetHumanLikeHeaders();
    
    for (const auto& header : humanHeaders) {
        std::string headerStr = header.first + ": " + header.second;
        headers = curl_slist_append(headers, headerStr.c_str());
    }
    
    curl_easy_setopt(handle, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(handle, CURLOPT_URL, url.c_str());
    curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(handle, CURLOPT_WRITEDATA, &callbackData);
    
    logger->Debug("Downloading page (human-like, max " + 
                 std::to_string(maxHtmlSize / 1024) + " KB)", "WebScraper");
    
    CURLcode res = curl_easy_perform(handle);
    
    curl_slist_free_all(headers);
    
    if (res != CURLE_OK) {
        logger->Error("cURL failed: " + std::string(curl_easy_strerror(res)), "WebScraper");
        return "";
    }
    
    long httpCode = 0;
    curl_easy_getinfo(handle, CURLINFO_RESPONSE_CODE, &httpCode);
    
    logger->Debug("HTTP " + std::to_string(httpCode) + 
                 ", Downloaded " + std::to_string(readBuffer.length() / 1024) + " KB" +
                 (foundMarker ? " (live marker found!)" : ""), "WebScraper");
    
    if (httpCode != Constants::HTTP_OK) {
        logger->Warning("Unexpected HTTP: " + std::to_string(httpCode), "WebScraper");
        return "";
    }
    
    humanBehavior->SimulatePageLoad();
    
    return readBuffer;
}


bool WebScraper::ParseStreamStatus(const std::string& html) {
    if (html.empty()) {
        logger->Warning("HTML empty, assuming offline", "WebScraper");
        return false;
    }
    
    if (html.length() < Constants::MIN_HTML_SIZE) {
        logger->Warning("HTML too small (" + std::to_string(html.length()) + " bytes)", "WebScraper");
        return false;
    }
    
    if (html.find("cf-browser-verification") != std::string::npos ||
        html.find("g-recaptcha") != std::string::npos) {
        logger->Warning("Anti-bot detected! Adjust headers if needed.", "WebScraper");
    }
    
    if (html.find(Constants::TwitchMarkers::IS_LIVE_BROADCAST) != std::string::npos) {
        logger->Info("Stream ONLINE (isLiveBroadcast)", "WebScraper");
        return true;
    }
    
    size_t streamPos = html.find(Constants::TwitchMarkers::STREAM_SECTION);
    if (streamPos != std::string::npos) {
        size_t searchEnd = std::min(streamPos + 500, html.length());
        std::string streamSection = html.substr(streamPos, searchEnd - streamPos);
        
        if (streamSection.find(Constants::TwitchMarkers::TYPE_LIVE) != std::string::npos) {
            logger->Info("Stream ONLINE (type:live)", "WebScraper");
            return true;
        }
    }
    
    if (html.find(Constants::TwitchMarkers::BROADCAST_TYPE) != std::string::npos) {
        logger->Info("Stream ONLINE (broadcastType)", "WebScraper");
        return true;
    }
    
    logger->Info("Stream OFFLINE", "WebScraper");
    return false;
}


bool WebScraper::CheckStreamStatus(const std::string& streamerName) {
    logger->Debug("Checking via web scraping (NO API)", "WebScraper");
    
    std::string html = DownloadPageHtml(streamerName);
    
    if (html.empty()) {
        logger->Warning("Could not download page, assuming offline", "WebScraper");
        return false;
    }
    
    return ParseStreamStatus(html);
}