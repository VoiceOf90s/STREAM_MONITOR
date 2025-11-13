#include "core/Config.h"
#include "StringUtils.h"
#include "Constants.h"
#include <iostream>
#include <algorithm>


Config::Config(const std::string& filePath) : configFilePath(filePath) {
}


bool Config::ParseBool(const std::string& value, bool defaultValue) const {
    std::string lowerValue = StringUtils::ToLower(value);
    
    if (lowerValue == "true" || lowerValue == "1" || 
        lowerValue == "yes" || lowerValue == "on") {
        return true;
    } 
    else if (lowerValue == "false" || lowerValue == "0" || 
             lowerValue == "no" || lowerValue == "off") {
        return false;
    }
    
    return defaultValue;
}


bool Config::ValidateSettings() {
    bool isValid = true;
    
    // Проверка интервалов
    int checkInterval = GetInt("check_interval", Constants::DEFAULT_CHECK_INTERVAL);
    if (checkInterval < 5 || checkInterval > 300) {
        std::cerr << "Warning: check_interval out of range (5-300), using default" << std::endl;
        Set("check_interval", std::to_string(Constants::DEFAULT_CHECK_INTERVAL));
        isValid = false;
    }
    
    // Проверка размера HTML
    int maxHtmlSize = GetInt("max_html_size", Constants::MAX_HTML_SIZE);
    if (maxHtmlSize < 10000 || maxHtmlSize > 1000000) {
        std::cerr << "Warning: max_html_size out of range, using default" << std::endl;
        Set("max_html_size", std::to_string(Constants::MAX_HTML_SIZE));
        isValid = false;
    }
    
    // Проверка таймаутов
    int timeout = GetInt("timeout", Constants::DEFAULT_TIMEOUT);
    if (timeout < 5 || timeout > 120) {
        std::cerr << "Warning: timeout out of range (5-120), using default" << std::endl;
        Set("timeout", std::to_string(Constants::DEFAULT_TIMEOUT));
        isValid = false;
    }
    
    return isValid;
}


bool Config::Load() {
    std::cout << "[Config] Load() START" << std::endl;
    std::cout << "[Config] Opening file: " << configFilePath << std::endl;
    
    std::ifstream file(configFilePath);
    
    if (!file.is_open()) {
        std::cerr << "[Config] Warning: Config file '" << configFilePath << "' not found. Creating default..." << std::endl;
        CreateDefault();
        std::cout << "[Config] Saving default config..." << std::endl;
        bool saved = Save();
        std::cout << "[Config] Save result: " << (saved ? "SUCCESS" : "FAILED") << std::endl;
        std::cout << "[Config] Load() END (created default)" << std::endl;
        return saved;
    }
    
    std::cout << "[Config] File opened successfully" << std::endl;
    std::cout << "[Config] Reading lines..." << std::endl;
    
    std::string line;
    int lineCount = 0;
    
    while (std::getline(file, line)) {
        lineCount++;
        
        if (lineCount % 10 == 0) {
            std::cout << "[Config] Processing line " << lineCount << "..." << std::endl;
        }
        
        // Убираем пробелы
        line = StringUtils::Trim(line);
        
        // Пропускаем комментарии и пустые строки
        if (line.empty() || line[0] == '#' || line[0] == ';') {
            continue;
        }
        
        // Парсинг формата key=value
        size_t delimiterPos = line.find('=');
        if (delimiterPos == std::string::npos) {
            std::cerr << "[Config] Warning: Invalid config line " << lineCount << ": " << line << std::endl;
            continue;
        }
        
        std::string key = StringUtils::Trim(line.substr(0, delimiterPos));
        std::string value = StringUtils::Trim(line.substr(delimiterPos + 1));
        
        if (!key.empty()) {
            settings[key] = value;
        }
    }
    
    file.close();
    
    std::cout << "[Config] File closed" << std::endl;
    std::cout << "[Config] Total lines read: " << lineCount << std::endl;
    std::cout << "[Config] Total settings loaded: " << settings.size() << std::endl;
    
    std::cout << "[Config] Validating settings..." << std::endl;
    ValidateSettings();
    std::cout << "[Config] Validation complete" << std::endl;
    
    std::cout << "[Config] Load() END (success)" << std::endl;
    return true;
}


bool Config::Save() {
    std::lock_guard<std::mutex> lock(configMutex);
    
    std::ofstream file(configFilePath);
    
    if (!file.is_open()) {
        std::cerr << "Error: Cannot create config file '" << configFilePath << "'" << std::endl;
        return false;
    }
    
    file << "# Twitch Stream Monitor Configuration v2.3" << std::endl;
    file << "# Auto-generated configuration file" << std::endl;
    file << "# Edit values, then restart the program" << std::endl;
    file << std::endl;
    
    file << "# Performance Settings" << std::endl;
    file << "check_interval=" << GetInt("check_interval", Constants::DEFAULT_CHECK_INTERVAL) << std::endl;
    file << "check_interval_fast=" << GetInt("check_interval_fast", Constants::FAST_CHECK_INTERVAL) << std::endl;
    file << "fast_mode_duration=" << GetInt("fast_mode_duration", Constants::FAST_MODE_DURATION) << std::endl;
    file << "max_html_size=" << GetInt("max_html_size", Constants::MAX_HTML_SIZE) << std::endl;
    file << std::endl;
    
    file << "# Network Settings" << std::endl;
    file << "timeout=" << GetInt("timeout", Constants::DEFAULT_TIMEOUT) << std::endl;
    file << "connect_timeout=" << GetInt("connect_timeout", Constants::DEFAULT_CONNECT_TIMEOUT) << std::endl;
    file << "use_http2=" << (GetBool("use_http2", true) ? "true" : "false") << std::endl;
    file << "dns_cache_timeout=" << GetInt("dns_cache_timeout", Constants::DNS_CACHE_TIMEOUT_SECONDS) << std::endl;
    file << "use_head_request=" << (GetBool("use_head_request", true) ? "true" : "false") << std::endl;
    file << std::endl;
    
    file << "# SSL Settings" << std::endl;
    file << "ssl_verify_peer=" << (GetBool("ssl_verify_peer", false) ? "true" : "false") << std::endl;
    file << "ssl_verify_host=" << (GetBool("ssl_verify_host", false) ? "true" : "false") << std::endl;
    file << std::endl;
    
    file << "# Logging Settings" << std::endl;
    file << "log_file=" << GetString("log_file", Constants::DEFAULT_LOG_FILE) << std::endl;
    file << "verbose_logging=" << (GetBool("verbose_logging", false) ? "true" : "false") << std::endl;
    file << std::endl;
    
    file << "# Browser Settings" << std::endl;
    file << "open_browser=" << (GetBool("open_browser", true) ? "true" : "false") << std::endl;
    file << "browser_delay_min=" << GetInt("browser_delay_min", Constants::BROWSER_DELAY_MIN_MS) << std::endl;
    file << "browser_delay_max=" << GetInt("browser_delay_max", Constants::BROWSER_DELAY_MAX_MS) << std::endl;
    file << "auto_close_tab=" << (GetBool("auto_close_tab", true) ? "true" : "false") << std::endl;
    file << std::endl;
    
    file << "# Features v2.2+" << std::endl;
    file << "enable_notifications=" << (GetBool("enable_notifications", true) ? "true" : "false") << std::endl;
    file << "enable_statistics=" << (GetBool("enable_statistics", true) ? "true" : "false") << std::endl;
    
    file.close();
    std::cout << "Configuration saved to '" << configFilePath << "'" << std::endl;
    return true;
}


void Config::CreateDefault() {
    // Performance Settings
    settings["check_interval"] = std::to_string(Constants::DEFAULT_CHECK_INTERVAL);
    settings["check_interval_fast"] = std::to_string(Constants::FAST_CHECK_INTERVAL);
    settings["fast_mode_duration"] = std::to_string(Constants::FAST_MODE_DURATION);
    settings["max_html_size"] = std::to_string(Constants::MAX_HTML_SIZE);
    
    // Network Settings
    settings["timeout"] = std::to_string(Constants::DEFAULT_TIMEOUT);
    settings["connect_timeout"] = std::to_string(Constants::DEFAULT_CONNECT_TIMEOUT);
    settings["use_http2"] = "true";
    settings["dns_cache_timeout"] = std::to_string(Constants::DNS_CACHE_TIMEOUT_SECONDS);
    settings["use_head_request"] = "true";
    
    // SSL Settings
    settings["ssl_verify_peer"] = "false";
    settings["ssl_verify_host"] = "false";
    
    // Logging Settings
    settings["log_file"] = Constants::DEFAULT_LOG_FILE;
    settings["verbose_logging"] = "false";
    
    // Browser Settings
    settings["open_browser"] = "true";
    settings["browser_delay_min"] = std::to_string(Constants::BROWSER_DELAY_MIN_MS);
    settings["browser_delay_max"] = std::to_string(Constants::BROWSER_DELAY_MAX_MS);
    settings["auto_close_tab"] = "true";
    
    // Features
    settings["enable_notifications"] = "true";
    settings["enable_statistics"] = "true";
}


std::string Config::GetString(const std::string& key, const std::string& defaultValue) const {
    std::lock_guard<std::mutex> lock(configMutex);
    
    auto it = settings.find(key);
    if (it != settings.end()) {
        return it->second;
    }
    return defaultValue;
}


int Config::GetInt(const std::string& key, int defaultValue) const {
    std::lock_guard<std::mutex> lock(configMutex);
    
    auto it = settings.find(key);
    if (it != settings.end()) {
        return StringUtils::SafeStoi(it->second, defaultValue);
    }
    return defaultValue;
}


bool Config::GetBool(const std::string& key, bool defaultValue) const {
    std::lock_guard<std::mutex> lock(configMutex);
    
    auto it = settings.find(key);
    if (it != settings.end()) {
        return ParseBool(it->second, defaultValue);
    }
    return defaultValue;
}


void Config::Set(const std::string& key, const std::string& value) {
    std::lock_guard<std::mutex> lock(configMutex);
    settings[key] = value;
}


bool Config::HasKey(const std::string& key) const {
    std::lock_guard<std::mutex> lock(configMutex);
    return settings.find(key) != settings.end();
}


std::vector<std::string> Config::GetAllKeys() const {
    std::lock_guard<std::mutex> lock(configMutex);
    
    std::vector<std::string> keys;
    keys.reserve(settings.size());
    
    for (const auto& pair : settings) {
        keys.push_back(pair.first);
    }
    
    return keys;
}