#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <string>


namespace Constants {
    // Network
    const int DEFAULT_TIMEOUT = 20;
    const int DEFAULT_CONNECT_TIMEOUT = 10;
    const int DNS_CACHE_TIMEOUT_SECONDS = 300;
    const long HTTP_OK = 200;
    
    // Check intervals
    const int DEFAULT_CHECK_INTERVAL = 30;
    const int FAST_CHECK_INTERVAL = 10;
    const int FAST_MODE_DURATION = 300;
    
    // HTML parsing
    const size_t MAX_HTML_SIZE = 100000;
    const size_t MIN_HTML_SIZE = 1000;
    
    // Browser (human-like delays)
    const int BROWSER_DELAY_MIN_MS = 1200;   // Увеличено для большей естественности
    const int BROWSER_DELAY_MAX_MS = 3500;   // Реальный человек не спешит
    
    // Human behavior simulation
    const int THINKING_DELAY_MIN_MS = 500;
    const int THINKING_DELAY_MAX_MS = 2000;
    const int TYPING_DELAY_MIN_MS = 100;
    const int TYPING_DELAY_MAX_MS = 300;
    const int MOUSE_MOVE_DELAY_MIN_MS = 50;
    const int MOUSE_MOVE_DELAY_MAX_MS = 150;
    const int PAGE_LOAD_WAIT_MIN_MS = 1000;
    const int PAGE_LOAD_WAIT_MAX_MS = 3000;
    
    // Anti-detection
    const int EXTRA_REQUEST_CHANCE_PERCENT = 20;  // 20% шанс доп. запроса
    const int REQUEST_INTERVAL_JITTER_MS = 500;   // Разброс времени между запросами
    
    // Twitch markers (для парсинга БЕЗ API)
    namespace TwitchMarkers {
        const char* const IS_LIVE_BROADCAST = "\"isLiveBroadcast\"";
        const char* const TYPE_LIVE = "\"type\":\"live\"";
        const char* const BROADCAST_TYPE = "\"broadcastType\":\"STREAM\"";
        const char* const STREAM_SECTION = "\"stream\":{";
        
        // Дополнительные маркеры для обнаружения
        const char* const LIVE_INDICATOR = "\"isLive\":true";
        const char* const VIEWER_COUNT = "\"viewersCount\"";
        const char* const STREAM_TITLE = "\"title\":";
    }
    
    // Anti-bot detection keywords
    namespace AntiBotKeywords {
        const char* const CLOUDFLARE = "cf-browser-verification";
        const char* const RECAPTCHA = "g-recaptcha";
        const char* const JS_REQUIRED = "Please enable JavaScript";
        const char* const BLOCKED = "Access Denied";
    }
    
    // File paths
    const char* const DEFAULT_CONFIG_FILE = "config.ini";
    const char* const DEFAULT_LOG_FILE = "stream_monitor.log";
    const char* const STREAMERS_LIST_FILE = "streamers.txt";
    
    // Regex patterns
    const char* const VALID_STREAMER_NAME_PATTERN = "^[a-zA-Z0-9_]{1,25}$";
    
    // URLs
    const char* const TWITCH_BASE_URL = "https://www.twitch.tv/";
    const char* const TWITCH_MAIN_PAGE = "https://www.twitch.tv/";
    const char* const TWITCH_DIRECTORY = "https://www.twitch.tv/directory";
    
    // Statistics
    const size_t MAX_SESSIONS_HISTORY = 1000;
    
    // Multi-monitor
    const int THREAD_START_DELAY_MS = 500;
}

#endif // CONSTANTS_H