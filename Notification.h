#ifndef NOTIFICATION_H
#define NOTIFICATION_H

#include <string>


// Класс для системных уведомлений (кроссплатформенный)
class Notification {
private:
    bool enabled;
    
    // Platform-specific уведомления
    void ShowWindowsNotification(const std::string& title, const std::string& message);
    void ShowLinuxNotification(const std::string& title, const std::string& message);
    void ShowMacOSNotification(const std::string& title, const std::string& message);

public:
    Notification(bool enable = true);
    
    // Показать уведомление
    void Show(const std::string& title, const std::string& message);
    
    // Уведомление о старте стрима
    void NotifyStreamOnline(const std::string& streamerName);
    
    // Уведомление об окончании стрима
    void NotifyStreamOffline(const std::string& streamerName);
    
    // Включить/выключить уведомления
    void SetEnabled(bool enable);
    bool IsEnabled() const { return enabled; }
};

#endif // NOTIFICATION_H