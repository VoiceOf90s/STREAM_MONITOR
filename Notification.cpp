#include "Notification.h"
#include <iostream>
#include <cstdlib>

#ifdef _WIN32
    #include <windows.h>
#endif


Notification::Notification(bool enable) : enabled(enable) {
}


void Notification::ShowWindowsNotification(const std::string& title, const std::string& message) {
#ifdef _WIN32
    // Используем PowerShell для Toast уведомлений
    std::string command = "powershell -Command \"$null = [Windows.UI.Notifications.ToastNotificationManager, Windows.UI.Notifications, ContentType = WindowsRuntime]; "
                         "$template = [Windows.UI.Notifications.ToastNotificationManager]::GetTemplateContent([Windows.UI.Notifications.ToastTemplateType]::ToastText02); "
                         "$toastXml = [xml]$template.GetXml(); "
                         "$toastXml.GetElementsByTagName('text')[0].AppendChild($toastXml.CreateTextNode('" + title + "')) | Out-Null; "
                         "$toastXml.GetElementsByTagName('text')[1].AppendChild($toastXml.CreateTextNode('" + message + "')) | Out-Null; "
                         "$xml = New-Object Windows.Data.Xml.Dom.XmlDocument; "
                         "$xml.LoadXml($toastXml.OuterXml); "
                         "$toast = [Windows.UI.Notifications.ToastNotification]::new($xml); "
                         "$toast.Tag = 'TwitchMonitor'; "
                         "$notifier = [Windows.UI.Notifications.ToastNotificationManager]::CreateToastNotifier('Twitch Stream Monitor'); "
                         "$notifier.Show($toast);\"";
    
    system(command.c_str());
#endif
}


void Notification::ShowLinuxNotification(const std::string& title, const std::string& message) {
#ifndef _WIN32
#ifndef __APPLE__
    // Используем notify-send на Linux
    std::string command = "notify-send \"" + title + "\" \"" + message + "\" -i video-display 2>/dev/null";
    int result = system(command.c_str());
    
    if (result != 0) {
        std::cerr << "Warning: notify-send not available. Install libnotify: sudo apt-get install libnotify-bin" << std::endl;
    }
#endif
#endif
}


void Notification::ShowMacOSNotification(const std::string& title, const std::string& message) {
#ifdef __APPLE__
    // Используем osascript на macOS
    std::string command = "osascript -e 'display notification \"" + message + 
                         "\" with title \"" + title + "\"'";
    system(command.c_str());
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