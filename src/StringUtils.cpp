#include "StringUtils.h"
#include <algorithm>
#include <cctype>


namespace StringUtils {

std::string Trim(const std::string& str) {
    if (str.empty()) {
        return "";
    }
    
    size_t first = str.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) {
        return "";
    }
    
    size_t last = str.find_last_not_of(" \t\r\n");
    return str.substr(first, (last - first + 1));
}


std::string ToLower(const std::string& str) {
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(),
                  [](unsigned char c) { return std::tolower(c); });
    return result;
}


bool IsValidStreamerName(const std::string& name) {
    if (name.empty() || name.length() > 25) {
        return false;
    }
    
    std::regex pattern("^[a-zA-Z0-9_]+$");
    return std::regex_match(name, pattern);
}


std::string EscapeShellArg(const std::string& arg) {
    std::string result;
    result.reserve(arg.length() * 2);
    
    for (char c : arg) {
        if (c == '"' || c == '\'' || c == '\\' || c == '$' || c == '`') {
            result += '\\';
        }
        result += c;
    }
    
    return result;
}


std::vector<std::string> Split(const std::string& str, char delimiter) {
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream tokenStream(str);
    
    while (std::getline(tokenStream, token, delimiter)) {
        tokens.push_back(token);
    }
    
    return tokens;
}


std::string Replace(const std::string& str, 
                   const std::string& from, 
                   const std::string& to) {
    std::string result = str;
    size_t pos = 0;
    
    while ((pos = result.find(from, pos)) != std::string::npos) {
        result.replace(pos, from.length(), to);
        pos += to.length();
    }
    
    return result;
}


int SafeStoi(const std::string& str, int defaultValue) {
    try {
        return std::stoi(str);
    } catch (...) {
        return defaultValue;
    }
}


long long SafeStoll(const std::string& str, long long defaultValue) {
    try {
        return std::stoll(str);
    } catch (...) {
        return defaultValue;
    }
}

} // namespace StringUtils