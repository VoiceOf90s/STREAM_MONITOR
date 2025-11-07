#include "Notification.h"
#include "StringUtils.h"
#include <iostream>
#include <cstdlib>

#ifdef _WIN32
    #include <windows.h>
#endif


Notification::Notification(bool enable) : enabled(enable) {
}


int Notification::ExecuteCommand(const std::string& command) const {
    int result = system(command.c_str());
    if (result != 0) {
        std::cerr << "Warning: Command execution failed with code " << result << std::endl;
    }
    return result;
}


void Notification::ShowWindowsNotification(const std::string& title, const std::string& message) {
#ifdef _WIN32
    // Экранирование кавычек для безопасности
    std::string safeTitle = StringUtils::Replace(title, "'", "''");
    std::string safeMessage = StringUtils::Replace(message, "'", "''");
    
    std::string command = "powershell -Command \"$null = [Windows.UI.Notifications.ToastNotificationManager, Windows.UI.Notifications, ContentType = WindowsRuntime]; "
                         "$template = [Windows.UI.Notifications.ToastNotificationManager]::GetTemplateContent([Windows.UI.Notifications.ToastTemplateType]::ToastText02); "
                         "$toastXml = [xml]$template.GetXml(); "
                         "$toastXml.GetElementsByTagName('text')[0].AppendChild($toastXml.CreateTextNode('" + safeTitle + "')) | Out-Null; "
                         "$toastXml.GetElementsByTagName('text')[1].AppendChild($toastXml.CreateTextNode('" + safeMessage + "')) | Out-Null; "
                         "$xml = New-Object Windows.Data.Xml.Dom.XmlDocument; "
                         "$xml.LoadXml($toastXml.OuterXml); "
                         "$toast = [Windows.UI.Notifications.ToastNotification]::new($xml); "
                         "$toast.Tag = 'TwitchMonitor'; "
                         "$notifier = [Windows.UI.Notifications.ToastNotificationManager]::CreateToastNotifier('Twitch Stream Monitor'); "
                         "$notifier.Show($toast);\"";
    
    ExecuteCommand(command);
#endif
}


void Notification::ShowLinuxNotification(const std::string& title, const std::string& message) {
#ifndef _WIN32
#ifndef __APPLE__
    // Экранирование для безопасности
    std::string safeTitle = StringUtils::EscapeShellArg(title);
    std::string safeMessage = StringUtils::EscapeShellArg(message);
    
    std::string command = "notify-send \"" + safeTitle + "\" \"" + safeMessage + "\" -i video-display 2>/dev/null";
    int result = ExecuteCommand(command);
    
    if (result != 0) {
        std::cerr << "Warning: notify-send not available. Install: sudo apt-get install libnotify-bin" << std::endl;
    }
#endif
#endif
}


void Notification::ShowMacOSNotification(const std::string& title, const std::string& message) {
#ifdef __APPLE__
    // Экранирование для AppleScript
    std::string safeTitle = StringUtils::Replace(title, "\"", "\\\"");
    std::string safeMessage = StringUtils::Replace(message, "\"", "\\\"");
    
    std::string command = "osascript -e 'display notification \"" + safeMessage + 
                         "\" with title \"" + safeTitle + "\"'";
    ExecuteCommand(command);
#endif
}


void Notification::Show(const std::string& title, const std::string& message) {
    if (!enabled) {
        return;
    }
    
    std::cout << "[NOTIFICATION] " << title << ": " << message << std::endl;
    
#ifdef _WIN32
    ShowWindowsNotification(title, message);
#elif __APPLE__
    ShowMacOSNotification(title, message);
#else
    ShowLinuxNotification(title, message);
#endif
}


void Notification::NotifyStreamOnline(const std::string& streamerName) {
    Show("Stream Started!", streamerName + " is now live on Twitch!");
}


void Notification::NotifyStreamOffline(const std::string& streamerName) {
    Show("Stream Ended", streamerName + " has gone offline.");
}


void Notification::SetEnabled(bool enable) {
    enabled = enable;
}