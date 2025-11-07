#ifndef NOTIFICATION_H
#define NOTIFICATION_H

#include <string>
#include <memory>


// Интерфейс для уведомлений (для тестирования)
class INotification {
public:
    virtual ~INotification() = default;
    virtual void Show(const std::string& title, const std::string& message) = 0;
    virtual void NotifyStreamOnline(const std::string& streamerName) = 0;
    virtual void NotifyStreamOffline(const std::string& streamerName) = 0;
    virtual void SetEnabled(bool enable) = 0;
    virtual bool IsEnabled() const = 0;
};


// Класс для системных уведомлений (кроссплатформенный)
class Notification : public INotification {
private:
    bool enabled;
    
    // Platform-specific уведомления
    void ShowWindowsNotification(const std::string& title, const std::string& message);
    void ShowLinuxNotification(const std::string& title, const std::string& message);
    void ShowMacOSNotification(const std::string& title, const std::string& message);
    
    // Безопасное выполнение shell команд
    int ExecuteCommand(const std::string& command) const;


public:
    explicit Notification(bool enable = true);
    
    // Показать уведомление
    void Show(const std::string& title, const std::string& message) override;
    
    // Уведомление о старте стрима
    void NotifyStreamOnline(const std::string& streamerName) override;
    
    // Уведомление об окончании стрима
    void NotifyStreamOffline(const std::string& streamerName) override;
    
    // Включить/выключить уведомления
    void SetEnabled(bool enable) override;
    bool IsEnabled() const override { return enabled; }
};

#endif // NOTIFICATION_H