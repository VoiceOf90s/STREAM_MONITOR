#ifndef MONITOR_CONTROLLER_H
#define MONITOR_CONTROLLER_H

#include <QObject>
#include <memory>
#include <atomic>
#include <thread>
#include "StreamerPriority.h"
#include "Config.h"
#include "Logger.h"
#include "WebScraper.h"
#include "BrowserController.h"


// Состояния мониторинга
enum class MonitorState {
  STOPPED,   // Программа остановлена
  RUNNING,   // Программа работает (проверяет и открывает)
  PAUSED     // Программа на паузе (только проверяет, не открывает)
};


// Контроллер мониторинга - связывает core логику с GUI
class MonitorController : public QObject {
  Q_OBJECT

private:
  std::unique_ptr<StreamerPriorityManager> priorityManager;
  std::shared_ptr<Config> config;
  std::shared_ptr<Logger> logger;
  std::unique_ptr<WebScraper> webScraper;
  std::unique_ptr<BrowserController> browserController;
  
  std::atomic<MonitorState> currentState;
  std::unique_ptr<std::thread> monitorThread;
  std::atomic<bool> shouldStop;
  
  int checkInterval;
  const int MAX_OPEN_TABS = 2;
  
  void MonitorLoop();
  void ProcessStreamers();
  void OpenStreamIfNeeded(const std::string& streamerName);
  void CloseTabIfNeeded(const std::string& streamerName);


public:
  explicit MonitorController(QObject* parent = nullptr);
  ~MonitorController();
  
  // Управление состоянием
  void Start();
  void Stop();
  void Pause();
  void Resume();
  
  MonitorState GetState() const { return currentState.load(); }
  bool IsRunning() const { return currentState.load() != MonitorState::STOPPED; }
  bool IsPaused() const { return currentState.load() == MonitorState::PAUSED; }
  
  // Управление стримерами
  bool AddStreamer(const std::string& name, int priority = -1);
  bool RemoveStreamer(const std::string& name);
  bool MoveStreamerUp(const std::string& name);
  bool MoveStreamerDown(const std::string& name);
  
  // Получение данных
  std::vector<StreamerPriority> GetAllStreamers() const;
  StreamerPriorityManager* GetPriorityManager() { return priorityManager.get(); }
  
  // Статистика
  int GetOpenTabsCount() const;
  std::vector<std::string> GetOnlineStreamers() const;


signals:
  // Сигналы для обновления GUI
  void streamerStatusChanged(QString name, StreamStatus status);
  void streamerTabOpenedChanged(QString name, bool opened);
  void monitoringStarted();
  void monitoringStopped();
  void monitoringPaused();
  void monitoringResumed();
  void errorOccurred(QString errorMessage);
  void logMessage(QString message);
};

#endif // MONITOR_CONTROLLER_H