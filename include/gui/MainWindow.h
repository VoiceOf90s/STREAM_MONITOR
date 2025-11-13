#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include <QMainWindow>
#include <QPushButton>
#include <QVBoxLayout>
#include <QLabel>
#include <QStatusBar>
#include <memory>
#include "MonitorController.h"
#include "StreamerListWidget.h"
#include "TrayIcon.h"


// Главное окно приложения
class MainWindow : public QMainWindow {
  Q_OBJECT

private:
  std::unique_ptr<MonitorController> controller;
  
  // UI элементы
  StreamerListWidget* streamerList;
  QPushButton* startStopButton;
  QPushButton* pauseResumeButton;
  QPushButton* addStreamerButton;
  QLabel* statusLabel;
  QLabel* openTabsLabel;
  
  std::unique_ptr<TrayIcon> trayIcon;
  
  // Настройка UI
  void SetupUI();
  void SetupConnections();
  void SetupMenuBar();
  void UpdateButtonStates();
  void LoadStyleSheet();
  
  // Обработка закрытия
  void closeEvent(QCloseEvent* event) override;


private slots:
  void OnStartStopClicked();
  void OnPauseResumeClicked();
  void OnAddStreamerClicked();
  void OnRemoveStreamerClicked();
  void OnMoveUpClicked();
  void OnMoveDownClicked();
  
  void OnStreamerStatusChanged(QString name, StreamStatus status);
  void OnStreamerTabOpenedChanged(QString name, bool opened);
  void OnMonitoringStarted();
  void OnMonitoringStopped();
  void OnMonitoringPaused();
  void OnMonitoringResumed();
  void OnErrorOccurred(QString errorMessage);
  
  void OnTrayIconActivated(QSystemTrayIcon::ActivationReason reason);
  void ShowWindow();
  void QuitApplication();


public:
  explicit MainWindow(QWidget* parent = nullptr);
  ~MainWindow();
};

#endif // MAIN_WINDOW_H