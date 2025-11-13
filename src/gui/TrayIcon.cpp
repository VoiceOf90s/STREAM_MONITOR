#include "TrayIcon.h"
#include <QApplication>
#include <QStyle>


TrayIcon::TrayIcon(QObject* parent)
  : QSystemTrayIcon(parent) {
  
  setIcon(QApplication::style()->standardIcon(QStyle::SP_ComputerIcon));
  setToolTip("Twitch Stream Monitor v2.4");
  
  CreateMenu();
  
  connect(this, &QSystemTrayIcon::activated, [this](ActivationReason reason) {
    if (reason == Trigger || reason == DoubleClick) {
      emit showWindowRequested();
    }
  });
}


void TrayIcon::CreateMenu() {
  trayMenu = new QMenu();
  
  showAction = new QAction("Show Window", this);
  connect(showAction, &QAction::triggered, this, &TrayIcon::showWindowRequested);
  
  pauseAction = new QAction("Pause", this);
  connect(pauseAction, &QAction::triggered, this, &TrayIcon::pauseToggleRequested);
  
  quitAction = new QAction("Quit", this);
  connect(quitAction, &QAction::triggered, this, &TrayIcon::quitRequested);
  
  trayMenu->addAction(showAction);
  trayMenu->addSeparator();
  trayMenu->addAction(pauseAction);
  trayMenu->addSeparator();
  trayMenu->addAction(quitAction);
  
  setContextMenu(trayMenu);
}


void TrayIcon::UpdatePauseAction(bool isPaused) {
  if (isPaused) {
    pauseAction->setText("Resume");
  } else {
    pauseAction->setText("Pause");
  }
}


void TrayIcon::ShowNotification(const QString& title, const QString& message) {
  showMessage(title, message, QSystemTrayIcon::Information, 3000);
}