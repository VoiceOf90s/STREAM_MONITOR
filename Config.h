#ifndef CONFIG_H
#define CONFIG_H

#include <string>
#include <fstream>
#include <sstream>
#include <map>


// Класс для работы с конфигурационным файлом
class Config {
private:
    std::map<std::string, std::string> settings;
    std::string configFilePath;
    
    // Удаление пробелов с краев строки
    std::string Trim(const std::string& str);
    
    // Парсинг значения как числа
    int ParseInt(const std::string& value, int defaultValue);
    
    // Парсинг значения как bool
    bool ParseBool(const std::string& value, bool defaultValue);

public:
    Config(const std::string& filePath = "config.ini");
    
    // Загрузка конфигурации из файла
    bool Load();
    
    // Сохранение конфигурации в файл
    bool Save();
    
    // Создание дефолтного конфига
    void CreateDefault();
    
    // Получение строкового значения
    std::string GetString(const std::string& key, const std::string& defaultValue = "");
    
    // Получение числового значения
    int GetInt(const std::string& key, int defaultValue);
    
    // Получение булевого значения
    bool GetBool(const std::string& key, bool defaultValue);
    
    // Установка значения
    void Set(const std::string& key, const std::string& value);
    
    // Проверка существования ключа
    bool HasKey(const std::string& key);
};

#endif // CONFIG_H