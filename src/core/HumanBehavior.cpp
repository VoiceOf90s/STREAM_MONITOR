#include "HumanBehavior.h"
#include <vector>
#include <algorithm>

HumanBehavior::HumanBehavior() {
    std::random_device rd;
    randomGenerator.seed(rd());
}


int HumanBehavior::GenerateThinkingDelay() const {
    std::uniform_int_distribution<> dist(500, 2000);
    return dist(const_cast<std::mt19937&>(randomGenerator));
}


int HumanBehavior::GenerateTypingDelay() const {
    std::uniform_int_distribution<> dist(100, 300);
    return dist(const_cast<std::mt19937&>(randomGenerator));
}


int HumanBehavior::GenerateMouseMoveDelay() const {
    std::uniform_int_distribution<> dist(50, 150);
    return dist(const_cast<std::mt19937&>(randomGenerator));
}


int HumanBehavior::GeneratePageLoadWait() const {
    std::uniform_int_distribution<> dist(1000, 3000);
    return dist(const_cast<std::mt19937&>(randomGenerator));
}


std::string HumanBehavior::GetRandomUserAgent() const {
    std::vector<std::string> userAgents = {
        // Chrome на Windows
        "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/120.0.0.0 Safari/537.36",
        "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/119.0.0.0 Safari/537.36",
        "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/121.0.0.0 Safari/537.36",
        
        // Firefox на Windows
        "Mozilla/5.0 (Windows NT 10.0; Win64; x64; rv:121.0) Gecko/20100101 Firefox/121.0",
        "Mozilla/5.0 (Windows NT 10.0; Win64; x64; rv:120.0) Gecko/20100101 Firefox/120.0",
        
        // Chrome на macOS
        "Mozilla/5.0 (Macintosh; Intel Mac OS X 10_15_7) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/120.0.0.0 Safari/537.36",
        "Mozilla/5.0 (Macintosh; Intel Mac OS X 10_15_7) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/119.0.0.0 Safari/537.36",
        
        // Safari на macOS
        "Mozilla/5.0 (Macintosh; Intel Mac OS X 10_15_7) AppleWebKit/605.1.15 (KHTML, like Gecko) Version/17.1 Safari/605.1.15",
        
        // Edge на Windows
        "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/120.0.0.0 Safari/537.36 Edg/120.0.0.0",
        
        // Chrome на Linux
        "Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/120.0.0.0 Safari/537.36"
    };
    
    std::uniform_int_distribution<> dist(0, userAgents.size() - 1);
    return userAgents[dist(const_cast<std::mt19937&>(randomGenerator))];
}


std::string HumanBehavior::GetRandomAcceptLanguage() const {
    std::vector<std::string> languages = {
        "en-US,en;q=0.9",
        "en-GB,en;q=0.9,en-US;q=0.8",
        "en-US,en;q=0.9,es;q=0.8",
        "en-US,en;q=0.9,de;q=0.8",
        "en-US,en;q=0.9,fr;q=0.8",
        "ru-RU,ru;q=0.9,en;q=0.8",
        "de-DE,de;q=0.9,en;q=0.8",
        "fr-FR,fr;q=0.9,en;q=0.8"
    };
    
    std::uniform_int_distribution<> dist(0, languages.size() - 1);
    return languages[dist(const_cast<std::mt19937&>(randomGenerator))];
}


std::string HumanBehavior::GetRandomReferer() const {
    std::vector<std::string> referers = {
        "https://www.google.com/",
        "https://www.twitch.tv/",
        "https://www.twitch.tv/directory",
        "https://www.youtube.com/",
        "https://twitter.com/",
        "https://www.reddit.com/r/Twitch/",
        ""  // Иногда без referer
    };
    
    std::uniform_int_distribution<> dist(0, referers.size() - 1);
    return referers[dist(const_cast<std::mt19937&>(randomGenerator))];
}


void HumanBehavior::SimulateThinking() const {
    int delayMs = GenerateThinkingDelay();
    std::this_thread::sleep_for(std::chrono::milliseconds(delayMs));
}


void HumanBehavior::SimulateTyping(const std::string& text) const {
    for (size_t i = 0; i < text.length(); ++i) {
        int delayMs = GenerateTypingDelay();
        std::this_thread::sleep_for(std::chrono::milliseconds(delayMs));
    }
}


void HumanBehavior::SimulateMouseMove() const {
    int delayMs = GenerateMouseMoveDelay();
    std::this_thread::sleep_for(std::chrono::milliseconds(delayMs));
}


void HumanBehavior::SimulatePageLoad() const {
    int delayMs = GeneratePageLoadWait();
    std::this_thread::sleep_for(std::chrono::milliseconds(delayMs));
}


void HumanBehavior::RandomDelay(int minMs, int maxMs) const {
    std::uniform_int_distribution<> dist(minMs, maxMs);
    int delayMs = dist(const_cast<std::mt19937&>(randomGenerator));
    std::this_thread::sleep_for(std::chrono::milliseconds(delayMs));
}


std::string HumanBehavior::GetSessionUserAgent() const {
    // Для всей сессии используем один и тот же User-Agent
    static std::string sessionUA = GetRandomUserAgent();
    return sessionUA;
}


std::vector<std::pair<std::string, std::string>> HumanBehavior::GetHumanLikeHeaders() const {
    std::vector<std::pair<std::string, std::string>> headers;
    
    // Обязательные заголовки
    headers.push_back({"User-Agent", GetSessionUserAgent()});
    headers.push_back({"Accept", "text/html,application/xhtml+xml,application/xml;q=0.9,image/avif,image/webp,image/apng,*/*;q=0.8"});
    headers.push_back({"Accept-Language", GetRandomAcceptLanguage()});
    headers.push_back({"Accept-Encoding", "gzip, deflate, br"});
    headers.push_back({"DNT", "1"});  // Do Not Track
    headers.push_back({"Connection", "keep-alive"});
    headers.push_back({"Upgrade-Insecure-Requests", "1"});
    headers.push_back({"Sec-Fetch-Dest", "document"});
    headers.push_back({"Sec-Fetch-Mode", "navigate"});
    headers.push_back({"Sec-Fetch-Site", "none"});
    headers.push_back({"Sec-Fetch-User", "?1"});
    headers.push_back({"Cache-Control", "max-age=0"});
    
    // Иногда добавляем Referer
    std::string referer = GetRandomReferer();
    if (!referer.empty()) {
        headers.push_back({"Referer", referer});
    }
    
    // Перемешиваем порядок (кроме User-Agent, который всегда первый)
    std::shuffle(headers.begin() + 1, headers.end(), 
                const_cast<std::mt19937&>(randomGenerator));
    
    return headers;
}


bool HumanBehavior::ShouldMakeExtraRequest() const {
    std::uniform_int_distribution<> dist(1, 100);
    return dist(const_cast<std::mt19937&>(randomGenerator)) <= 20;  // 20% шанс
}