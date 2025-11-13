#include "MonitorController.h"
#include "Constants.h"
#include <QDebug>
#include <chrono>
#include <thread>


MonitorController::MonitorController(QObject* parent)
  : QObject(parent), currentState(MonitorState::STOPPED), shouldStop(false) {
  
  qDebug() << "[MonitorController] Initializing...";
  
  priorityManager = std::make_unique<StreamerPriorityManager>("streamers.txt");
  
  config = std::make_shared<Config>("config.ini");
  config->Load();
  
  logger = std::make_shared<Logger>(
    config->GetString("log_file", Constants::DEFAULT_LOG_FILE),
    config->GetBool("verbose_logging", false)
  );
  
  webScraper = std::make_unique<WebScraper>(logger, *config);
  browserController = std::make_unique<BrowserController>(logger, true);
  
  checkInterval = config->GetInt("check_interval", Constants::DEFAULT_CHECK_INTERVAL);
  
  logger->System("MonitorController initialized", "MonitorController");
  qDebug() << "[MonitorController] Initialization complete";
}


MonitorController::~MonitorController() {
  Stop();
  logger->System("MonitorController destroyed", "MonitorController");
}


void MonitorController::Start() {
  if (currentState.load() != MonitorState::STOPPED) {
    qWarning() << "[MonitorController] Already running, cannot start";
    return;
  }
  
  qDebug() << "[MonitorController] Starting monitoring...";
  logger->System("Starting monitoring system", "MonitorController");
  
  shouldStop = false;
  currentState = MonitorState::RUNNING;
  
  monitorThread = std::make_unique<std::thread>(&MonitorController::MonitorLoop, this);
  
  emit monitoringStarted();
  emit logMessage("Monitoring started");
}


void MonitorController::Stop() {
  if (currentState.load() == MonitorState::STOPPED) {
    return;
  }
  
  qDebug() << "[MonitorController] Stopping monitoring...";
  logger->System("Stopping monitoring system", "MonitorController");
  
  shouldStop = true;
  currentState = MonitorState::STOPPED;
  
  if (monitorThread && monitorThread->joinable()) {
    monitorThread->join();
  }
  
  emit monitoringStopped();
  emit logMessage("Monitoring stopped");
}


void MonitorController::Pause() {
  if (currentState.load() != MonitorState::RUNNING) {
    return;
  }
  
  qDebug() << "[MonitorController] Pausing (check only mode)";
  logger->System("Pausing monitoring (check only)", "MonitorController");
  
  currentState = MonitorState::PAUSED;
  
  emit monitoringPaused();
  emit logMessage("Monitoring paused (checking only, not opening tabs)");
}


void MonitorController::Resume() {
  if (currentState.load() != MonitorState::PAUSED) {
    return;
  }
  
  qDebug() << "[MonitorController] Resuming monitoring";
  logger->System("Resuming monitoring", "MonitorController");
  
  currentState = MonitorState::RUNNING;
  
  emit monitoringResumed();
  emit logMessage("Monitoring resumed");
}


void MonitorController::MonitorLoop() {
  logger->Info("Monitor loop started", "MonitorController");
  
  while (!shouldStop.load()) {
    try {
      ProcessStreamers();
      
    } catch (const std::exception& e) {
      std::string error = "Exception in monitor loop: " + std::string(e.what());
      logger->Error(error, "MonitorController");
      emit errorOccurred(QString::fromStdString(error));
    }
    
    for (int i = 0; i < checkInterval && !shouldStop.load(); i++) {
      std::this_thread::sleep_for(std::chrono::seconds(1));
    }
  }
  
  logger->Info("Monitor loop stopped", "MonitorController");
}


void MonitorController::ProcessStreamers() {
  std::vector<std::string> streamersToCheck = priorityManager->GetStreamersToCheck();
  
  qDebug() << "[MonitorController] Checking" << streamersToCheck.size() << "streamers";
  logger->Debug("Checking " + std::to_string(streamersToCheck.size()) + " streamers", 
               "MonitorController");
  
  for (const auto& streamerName : streamersToCheck) {
    if (shouldStop.load()) {
      break;
    }
    
    logger->Debug("Checking: " + streamerName, "MonitorController");
    
    StreamerPriority* streamer = priorityManager->GetStreamer(streamerName);
    if (!streamer) {
      continue;
    }
    
    streamer->SetStatus(StreamStatus::CHECKING);
    emit streamerStatusChanged(QString::fromStdString(streamerName), StreamStatus::CHECKING);
    
    bool isOnline = webScraper->CheckStreamStatus(streamerName);
    
    StreamStatus newStatus = isOnline ? StreamStatus::ONLINE : StreamStatus::OFFLINE;
    StreamStatus oldStatus = streamer->GetStatus();
    
    streamer->SetStatus(newStatus);
    streamer->UpdateLastCheckTime();
    
    emit streamerStatusChanged(QString::fromStdString(streamerName), newStatus);
    
    if (newStatus == StreamStatus::ONLINE && oldStatus != StreamStatus::ONLINE) {
      logger->Event("Stream went online: " + streamerName, "MonitorController");
      
      if (currentState.load() == MonitorState::RUNNING) {
        OpenStreamIfNeeded(streamerName);
      }
    }
    else if (newStatus == StreamStatus::OFFLINE && oldStatus == StreamStatus::ONLINE) {
      logger->Event("Stream went offline: " + streamerName, "MonitorController");
      CloseTabIfNeeded(streamerName);
    }
  }
}


void MonitorController::OpenStreamIfNeeded(const std::string& streamerName) {
  int openTabs = priorityManager->GetOpenTabsCount();
  
  qDebug() << "[MonitorController] Open tabs:" << openTabs << "/ MAX:" << MAX_OPEN_TABS;
  
  if (openTabs >= MAX_OPEN_TABS) {
    logger->Warning("Cannot open tab for " + streamerName + ": limit reached (" + 
                   std::to_string(openTabs) + "/" + std::to_string(MAX_OPEN_TABS) + ")",
                   "MonitorController");
    
    emit logMessage(QString("Cannot open %1: tab limit reached (%2/%3)")
                   .arg(QString::fromStdString(streamerName))
                   .arg(openTabs)
                   .arg(MAX_OPEN_TABS));
    return;
  }
  
  StreamerPriority* streamer = priorityManager->GetStreamer(streamerName);
  if (!streamer || streamer->IsTabOpened()) {
    return;
  }
  
  logger->Success("Opening stream in browser: " + streamerName, "MonitorController");
  browserController->OpenStream(streamerName);
  
  streamer->SetTabOpened(true);
  streamer->SetOnlineDetectedTime(
    std::chrono::duration_cast<std::chrono::seconds>(
      std::chrono::system_clock::now().time_since_epoch()
    ).count()
  );
  
  emit streamerTabOpenedChanged(QString::fromStdString(streamerName), true);
  emit logMessage(QString("Opened %1 stream (%2/%3 tabs)")
                 .arg(QString::fromStdString(streamerName))
                 .arg(openTabs + 1)
                 .arg(MAX_OPEN_TABS));
}


void MonitorController::CloseTabIfNeeded(const std::string& streamerName) {
  StreamerPriority* streamer = priorityManager->GetStreamer(streamerName);
  if (!streamer || !streamer->IsTabOpened()) {
    return;
  }
  
  logger->Info("Stream offline, closing tab: " + streamerName, "MonitorController");
  
  streamer->SetTabOpened(false);
  
  emit streamerTabOpenedChanged(QString::fromStdString(streamerName), false);
  emit logMessage(QString("%1 went offline (tab closed)")
                 .arg(QString::fromStdString(streamerName)));
  
  std::vector<std::string> onlineStreamers = priorityManager->GetOnlineStreamers();
  int openTabs = priorityManager->GetOpenTabsCount();
  
  if (openTabs < MAX_OPEN_TABS && !onlineStreamers.empty()) {
    for (const auto& nextStreamer : onlineStreamers) {
      StreamerPriority* candidate = priorityManager->GetStreamer(nextStreamer);
      if (candidate && !candidate->IsTabOpened()) {
        OpenStreamIfNeeded(nextStreamer);
        break;
      }
    }
  }
}


bool MonitorController::AddStreamer(const std::string& name, int priority) {
  bool success = priorityManager->AddStreamer(name, priority);
  
  if (success) {
    logger->Success("Added streamer: " + name + " (priority: " + std::to_string(priority) + ")",
                   "MonitorController");
    emit logMessage(QString("Added streamer: %1").arg(QString::fromStdString(name)));
  } else {
    logger->Warning("Failed to add streamer: " + name, "MonitorController");
  }
  
  return success;
}


bool MonitorController::RemoveStreamer(const std::string& name) {
  bool success = priorityManager->RemoveStreamer(name);
  
  if (success) {
    logger->Success("Removed streamer: " + name, "MonitorController");
    emit logMessage(QString("Removed streamer: %1").arg(QString::fromStdString(name)));
  }
  
  return success;
}


bool MonitorController::MoveStreamerUp(const std::string& name) {
  bool success = priorityManager->MoveUp(name);
  
  if (success) {
    logger->Info("Moved up: " + name, "MonitorController");
    emit logMessage(QString("Moved up: %1").arg(QString::fromStdString(name)));
  }
  
  return success;
}


bool MonitorController::MoveStreamerDown(const std::string& name) {
  bool success = priorityManager->MoveDown(name);
  
  if (success) {
    logger->Info("Moved down: " + name, "MonitorController");
    emit logMessage(QString("Moved down: %1").arg(QString::fromStdString(name)));
  }
  
  return success;
}


std::vector<StreamerPriority> MonitorController::GetAllStreamers() const {
  return priorityManager->GetAllStreamers();
}


int MonitorController::GetOpenTabsCount() const {
  return priorityManager->GetOpenTabsCount();
}


std::vector<std::string> MonitorController::GetOnlineStreamers() const {
  return priorityManager->GetOnlineStreamers();
}