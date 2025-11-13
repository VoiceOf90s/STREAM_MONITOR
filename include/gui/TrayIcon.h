#ifndef TRAY_ICON_H
#define TRAY_ICON_H

#include <QSystemTrayIcon>
#include <QMenu>
#include <QAction>


// Иконка в системном трее
class TrayIcon : public QSystemTrayIcon {
  Q_OBJECT

private:
  QMenu* trayMenu;
  QAction* showAction;
  QAction* pauseAction;
  QAction* quitAction;
  
  void CreateMenu();


public:
  explicit TrayIcon(QObject* parent = nullptr);
  
  void UpdatePauseAction(bool isPaused);
  void ShowNotification(const QString& title, const QString& message);


signals:
  void showWindowRequested();
  void pauseToggleRequested();
  void quitRequested();
};

#endif // TRAY_ICON_H