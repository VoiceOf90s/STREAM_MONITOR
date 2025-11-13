#ifndef CONFIG_H
#define CONFIG_H

#include <string>
#include <fstream>
#include <map>
#include <vector> 
#include <mutex>


class Config {
private:
    std::map<std::string, std::string> settings;
    std::string configFilePath;
    mutable std::mutex configMutex;  // Для thread-safety
    
    // Валидация значений
    bool ValidateSettings();
    
    // Парсинг булевого значения (безопасный)
    bool ParseBool(const std::string& value, bool defaultValue) const;


public:
    Config(const std::string& filePath = "config.ini");
    
    // Загрузка конфигурации из файла
    bool Load();
    
    // Сохранение конфигурации в файл
    bool Save();
    
    // Создание дефолтного конфига
    void CreateDefault();
    
    // Получение строкового значения (thread-safe)
    std::string GetString(const std::string& key, 
                         const std::string& defaultValue = "") const;
    
    // Получение числового значения (thread-safe)
    int GetInt(const std::string& key, int defaultValue) const;
    
    // Получение булевого значения (thread-safe)
    bool GetBool(const std::string& key, bool defaultValue) const;
    
    // Установка значения (thread-safe)
    void Set(const std::string& key, const std::string& value);
    
    // Проверка существования ключа (thread-safe)
    bool HasKey(const std::string& key) const;
    
    // Получение всех ключей
    std::vector<std::string> GetAllKeys() const;
};

#endif // CONFIG_H