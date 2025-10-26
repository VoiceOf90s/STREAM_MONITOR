#include "Config.h"
#include <iostream>
#include <algorithm>


Config::Config(const std::string& filePath) : configFilePath(filePath) {
}


std::string Config::Trim(const std::string& str) {
    size_t first = str.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) return "";
    
    size_t last = str.find_last_not_of(" \t\r\n");
    return str.substr(first, (last - first + 1));
}


int Config::ParseInt(const std::string& value, int defaultValue) {
    try {
        return std::stoi(value);
    } catch (...) {
        return defaultValue;
    }
}


bool Config::ParseBool(const std::string& value, bool defaultValue) {
    std::string lowerValue = value;
    std::transform(lowerValue.begin(), lowerValue.end(), lowerValue.begin(), ::tolower);
    
    if (lowerValue == "true" || lowerValue == "1" || lowerValue == "yes" || lowerValue == "on") {
        return true;
    } else if (lowerValue == "false" || lowerValue == "0" || lowerValue == "no" || lowerValue == "off") {
        return false;
    }
    
    return defaultValue;
}


bool Config::Load() {
    std::ifstream file(configFilePath);
    
    if (!file.is_open()) {
        std::cerr << "Warning: Config file '" << configFilePath << "' not found. Creating default..." << std::endl;
        CreateDefault();
        return Save();
    }
    
    std::string line;
    while (std::getline(file, line)) {
        // Пропускаем комментарии и пустые строки
        line = Trim(line);
        if (line.empty() || line[0] == '#' || line[0] == ';') {
            continue;
        }
        
        // Парсинг формата key=value
        size_t delimiterPos = line.find('=');
        if (delimiterPos != std::string::npos) {
            std::string key = Trim(line.substr(0, delimiterPos));
            std::string value = Trim(line.substr(delimiterPos + 1));
            settings[key] = value;
        }
    }
    
    file.close();
    return true;
}


bool Config::Save() {
    std::ofstream file(configFilePath);
    
    if (!file.is_open()) {
        std::cerr << "Error: Cannot create config file '" << configFilePath << "'" << std::endl;
        return false;
    }
    
    file << "# Twitch Stream Monitor Configuration v2.1" << std::endl;
    file << "# Auto-generated configuration file" << std::endl;
    file << std::endl;
    
    file << "# Performance Settings" << std::endl;
    file << "check_interval=" << GetInt("check_interval", 30) << std::endl;
    file << "check_interval_fast=" << GetInt("check_interval_fast", 10) << std::endl;
    file << "fast_mode_duration=" << GetInt("fast_mode_duration", 300) << std::endl;
    file << "max_html_size=" << GetInt("max_html_size", 100000) << std::endl;
    file << std::endl;
    
    file << "# Network Settings" << std::endl;
    file << "timeout=" << GetInt("timeout", 20) << std::endl;
    file << "connect_timeout=" << GetInt("connect_timeout", 10) << std::endl;
    file << "use_http2=" << (GetBool("use_http2", true) ? "true" : "false") << std::endl;
    file << "dns_cache_timeout=" << GetInt("dns_cache_timeout", 300) << std::endl;
    file << "use_head_request=" << (GetBool("use_head_request", true) ? "true" : "false") << std::endl;
    file << std::endl;
    
    file << "# SSL Settings" << std::endl;
    file << "ssl_verify_peer=" << (GetBool("ssl_verify_peer", false) ? "true" : "false") << std::endl;
    file << "ssl_verify_host=" << (GetBool("ssl_verify_host", false) ? "true" : "false") << std::endl;
    file << std::endl;
    
    file << "# Logging Settings" << std::endl;
    file << "log_file=" << GetString("log_file", "stream_monitor.log") << std::endl;
    file << "verbose_logging=" << (GetBool("verbose_logging", false) ? "true" : "false") << std::endl;
    file << std::endl;
    
    file << "# Browser Settings" << std::endl;
    file << "open_browser=" << (GetBool("open_browser", true) ? "true" : "false") << std::endl;
    file << "browser_delay_min=" << GetInt("browser_delay_min", 800) << std::endl;
    file << "browser_delay_max=" << GetInt("browser_delay_max", 2000) << std::endl;
    file << "auto_close_tab=" << (GetBool("auto_close_tab", true) ? "true" : "false") << std::endl;
    
    file.close();
    std::cout << "Configuration saved to '" << configFilePath << "'" << std::endl;
    return true;
}


void Config::CreateDefault() {
    // Performance Settings
    settings["check_interval"] = "30";
    settings["check_interval_fast"] = "10";
    settings["fast_mode_duration"] = "300";
    settings["max_html_size"] = "100000";
    
    // Network Settings
    settings["timeout"] = "20";
    settings["connect_timeout"] = "10";
    settings["use_http2"] = "true";
    settings["dns_cache_timeout"] = "300";
    settings["use_head_request"] = "true";
    
    // SSL Settings
    settings["ssl_verify_peer"] = "false";
    settings["ssl_verify_host"] = "false";
    
    // Logging Settings
    settings["log_file"] = "stream_monitor.log";
    settings["verbose_logging"] = "false";
    
    // Browser Settings
    settings["open_browser"] = "true";
    settings["browser_delay_min"] = "800";
    settings["browser_delay_max"] = "2000";
}


std::string Config::GetString(const std::string& key, const std::string& defaultValue) {
    if (settings.find(key) != settings.end()) {
        return settings[key];
    }
    return defaultValue;
}


int Config::GetInt(const std::string& key, int defaultValue) {
    if (settings.find(key) != settings.end()) {
        return ParseInt(settings[key], defaultValue);
    }
    return defaultValue;
}


bool Config::GetBool(const std::string& key, bool defaultValue) {
    if (settings.find(key) != settings.end()) {
        return ParseBool(settings[key], defaultValue);
    }
    return defaultValue;
}


void Config::Set(const std::string& key, const std::string& value) {
    settings[key] = value;
}


bool Config::HasKey(const std::string& key) {
    return settings.find(key) != settings.end();
}