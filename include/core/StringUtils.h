#ifndef STRING_UTILS_H
#define STRING_UTILS_H

#include <string>
#include <vector>
#include <regex>


namespace StringUtils {
    // Удаление пробелов с краев
    std::string Trim(const std::string& str);
    
    // Приведение к нижнему регистру
    std::string ToLower(const std::string& str);
    
    // Проверка валидности имени стримера
    bool IsValidStreamerName(const std::string& name);
    
    // Экранирование для shell команд
    std::string EscapeShellArg(const std::string& arg);
    
    // Разделение строки по разделителю
    std::vector<std::string> Split(const std::string& str, char delimiter);
    
    // Замена подстроки
    std::string Replace(const std::string& str, 
                       const std::string& from, 
                       const std::string& to);
    
    // Безопасный stoi с дефолтным значением
    int SafeStoi(const std::string& str, int defaultValue);
    
    // Безопасный stoll с дефолтным значением
    long long SafeStoll(const std::string& str, long long defaultValue);
}

#endif // STRING_UTILS_H