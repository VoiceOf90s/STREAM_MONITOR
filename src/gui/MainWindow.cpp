#include "MainWindow.h"
#include "AddStreamerDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QMessageBox>
#include <QCloseEvent>
#include <QFile>
#include <QTextStream>
#include <QGroupBox>
#include <QSplitter>


MainWindow::MainWindow(QWidget* parent)
  : QMainWindow(parent) {
  
  setWindowTitle("Twitch Stream Monitor v2.4");
  setMinimumSize(800, 600);
  
  controller = std::make_unique<MonitorController>(this);
  
  SetupUI();
  SetupConnections();
  SetupMenuBar();
  LoadStyleSheet();
  
  trayIcon = std::make_unique<TrayIcon>(this);
  trayIcon->show();
  
  connect(trayIcon.get(), &TrayIcon::showWindowRequested, 
          this, &MainWindow::ShowWindow);
  connect(trayIcon.get(), &TrayIcon::pauseToggleRequested, 
          this, &MainWindow::OnPauseResumeClicked);
  connect(trayIcon.get(), &TrayIcon::quitRequested, 
          this, &MainWindow::QuitApplication);
  
  UpdateButtonStates();
}


MainWindow::~MainWindow() {
  controller->Stop();
}


void MainWindow::SetupUI() {
  QWidget* centralWidget = new QWidget(this);
  setCentralWidget(centralWidget);
  
  QVBoxLayout* mainLayout = new QVBoxLayout(centralWidget);
  mainLayout->setSpacing(10);
  mainLayout->setContentsMargins(15, 15, 15, 15);
  
  // Header с информацией
  QGroupBox* headerGroup = new QGroupBox("System Status", this);
  QHBoxLayout* headerLayout = new QHBoxLayout(headerGroup);
  
  statusLabel = new QLabel("Status: <b>Stopped</b>", this);
  statusLabel->setStyleSheet("font-size: 14px;");
  
  openTabsLabel = new QLabel("Open tabs: <b>0/2</b>", this);
  openTabsLabel->setStyleSheet("font-size: 14px;");
  
  headerLayout->addWidget(statusLabel);
  headerLayout->addStretch();
  headerLayout->addWidget(openTabsLabel);
  
  mainLayout->addWidget(headerGroup);
  
  // Список стримеров
  QGroupBox* streamersGroup = new QGroupBox("Streamers (ordered by priority)", this);
  QVBoxLayout* streamersLayout = new QVBoxLayout(streamersGroup);
  
  streamerList = new StreamerListWidget(this);
  streamersLayout->addWidget(streamerList);
  
  // Кнопки управления списком
  QHBoxLayout* listControlLayout = new QHBoxLayout();
  
  addStreamerButton = new QPushButton("➕ Add Streamer", this);
  addStreamerButton->setStyleSheet("font-weight: bold; padding: 8px;");
  
  QPushButton* removeButton = new QPushButton("🗑️ Remove", this);
  QPushButton* moveUpButton = new QPushButton("⬆️ Move Up", this);
  QPushButton* moveDownButton = new QPushButton("⬇️ Move Down", this);
  
  listControlLayout->addWidget(addStreamerButton);
  listControlLayout->addWidget(removeButton);
  listControlLayout->addStretch();
  listControlLayout->addWidget(moveUpButton);
  listControlLayout->addWidget(moveDownButton);
  
  streamersLayout->addLayout(listControlLayout);
  mainLayout->addWidget(streamersGroup);
  
  // Кнопки управления мониторингом
  QGroupBox* controlGroup = new QGroupBox("Monitoring Control", this);
  QHBoxLayout* controlLayout = new QHBoxLayout(controlGroup);
  
  startStopButton = new QPushButton("▶️ Start Monitoring", this);
  startStopButton->setStyleSheet(
    "QPushButton { font-size: 16px; font-weight: bold; padding: 12px; "
    "background-color: #4CAF50; color: white; border-radius: 5px; }"
    "QPushButton:hover { background-color: #45a049; }"
    "QPushButton:pressed { background-color: #3d8b40; }"
  );
  
  pauseResumeButton = new QPushButton("⏸️ Pause", this);
  pauseResumeButton->setEnabled(false);
  pauseResumeButton->setStyleSheet(
    "QPushButton { font-size: 16px; font-weight: bold; padding: 12px; "
    "background-color: #FF9800; color: white; border-radius: 5px; }"
    "QPushButton:hover { background-color: #e68900; }"
    "QPushButton:pressed { background-color: #cc7a00; }"
    "QPushButton:disabled { background-color: #cccccc; color: #666666; }"
  );
  
  controlLayout->addWidget(startStopButton);
  controlLayout->addWidget(pauseResumeButton);
  
  mainLayout->addWidget(controlGroup);
  
  // Status bar
  setStatusBar(new QStatusBar(this));
  statusBar()->showMessage("Ready");
  
  // Загрузка стримеров
  auto streamers = controller->GetAllStreamers();
  streamerList->UpdateStreamers(streamers);
  
  // Подключение кнопок управления списком
  connect(removeButton, &QPushButton::clicked, this, &MainWindow::OnRemoveStreamerClicked);
  connect(moveUpButton, &QPushButton::clicked, this, &MainWindow::OnMoveUpClicked);
  connect(moveDownButton, &QPushButton::clicked, this, &MainWindow::OnMoveDownClicked);
}


void MainWindow::SetupConnections() {
  // Кнопки
  connect(startStopButton, &QPushButton::clicked, this, &MainWindow::OnStartStopClicked);
  connect(pauseResumeButton, &QPushButton::clicked, this, &MainWindow::OnPauseResumeClicked);
  connect(addStreamerButton, &QPushButton::clicked, this, &MainWindow::OnAddStreamerClicked);
  
  // Сигналы от контроллера
  connect(controller.get(), &MonitorController::streamerStatusChanged,
          this, &MainWindow::OnStreamerStatusChanged);
  connect(controller.get(), &MonitorController::streamerTabOpenedChanged,
          this, &MainWindow::OnStreamerTabOpenedChanged);
  connect(controller.get(), &MonitorController::monitoringStarted,
          this, &MainWindow::OnMonitoringStarted);
  connect(controller.get(), &MonitorController::monitoringStopped,
          this, &MainWindow::OnMonitoringStopped);
  connect(controller.get(), &MonitorController::monitoringPaused,
          this, &MainWindow::OnMonitoringPaused);
  connect(controller.get(), &MonitorController::monitoringResumed,
          this, &MainWindow::OnMonitoringResumed);
  connect(controller.get(), &MonitorController::errorOccurred,
          this, &MainWindow::OnErrorOccurred);
  connect(controller.get(), &MonitorController::logMessage,
          [this](QString msg) { statusBar()->showMessage(msg, 5000); });
}


void MainWindow::SetupMenuBar() {
  QMenu* fileMenu = menuBar()->addMenu("&File");
  
  QAction* refreshAction = new QAction("🔄 Refresh List", this);
  connect(refreshAction, &QAction::triggered, [this]() {
    auto streamers = controller->GetAllStreamers();
    streamerList->UpdateStreamers(streamers);
    statusBar()->showMessage("List refreshed", 2000);
  });
  
  QAction* minimizeToTrayAction = new QAction("Minimize to Tray", this);
  connect(minimizeToTrayAction, &QAction::triggered, [this]() {
    hide();
    trayIcon->ShowNotification("Twitch Monitor", "Running in background");
  });
  
  QAction* quitAction = new QAction("Quit", this);
  quitAction->setShortcut(QKeySequence::Quit);
  connect(quitAction, &QAction::triggered, this, &MainWindow::QuitApplication);
  
  fileMenu->addAction(refreshAction);
  fileMenu->addSeparator();
  fileMenu->addAction(minimizeToTrayAction);
  fileMenu->addAction(quitAction);
  
  QMenu* helpMenu = menuBar()->addMenu("&Help");
  
  QAction* aboutAction = new QAction("About", this);
  connect(aboutAction, &QAction::triggered, [this]() {
    QMessageBox::about(this, "About Twitch Stream Monitor",
      "<h2>Twitch Stream Monitor v2.4</h2>"
      "<p><b>Features:</b></p>"
      "<ul>"
      "<li>Priority-based monitoring (max 2 tabs open)</li>"
      "<li>Automatic browser control</li>"
      "<li>System tray support</li>"
      "<li>Pause mode (check only, no opening)</li>"
      "</ul>"
      "<p><b>Contact:</b></p>"
      "<ul>"
      "<li>Email: chtonibud86@gmail.com</li>"
      "<li>Discord: grimoire3798</li>"
      "</ul>"
      "<p>Created with ❤️ for streamers and their fans</p>"
    );
  });
  
  helpMenu->addAction(aboutAction);
}


void MainWindow::LoadStyleSheet() {
  QFile styleFile(":/styles/dark_theme.qss");
  if (styleFile.open(QFile::ReadOnly | QFile::Text)) {
    QTextStream stream(&styleFile);
    QString style = stream.readAll();
    setStyleSheet(style);
    styleFile.close();
  }
}


void MainWindow::UpdateButtonStates() {
  MonitorState state = controller->GetState();
  
  if (state == MonitorState::STOPPED) {
    startStopButton->setText("▶️ Start Monitoring");
    startStopButton->setEnabled(true);
    startStopButton->setStyleSheet(
      "QPushButton { font-size: 16px; font-weight: bold; padding: 12px; "
      "background-color: #4CAF50; color: white; border-radius: 5px; }"
      "QPushButton:hover { background-color: #45a049; }"
    );
    
    pauseResumeButton->setEnabled(false);
    pauseResumeButton->setText("⏸️ Pause");
    
    addStreamerButton->setEnabled(true);
    streamerList->setEnabled(true);
    
  } else if (state == MonitorState::RUNNING) {
    startStopButton->setText("⏹️ Stop Monitoring");
    startStopButton->setStyleSheet(
      "QPushButton { font-size: 16px; font-weight: bold; padding: 12px; "
      "background-color: #f44336; color: white; border-radius: 5px; }"
      "QPushButton:hover { background-color: #da190b; }"
    );
    
    pauseResumeButton->setEnabled(true);
    pauseResumeButton->setText("⏸️ Pause");
    
    addStreamerButton->setEnabled(false);
    streamerList->setEnabled(false);
    
  } else if (state == MonitorState::PAUSED) {
    startStopButton->setText("⏹️ Stop Monitoring");
    startStopButton->setStyleSheet(
      "QPushButton { font-size: 16px; font-weight: bold; padding: 12px; "
      "background-color: #f44336; color: white; border-radius: 5px; }"
      "QPushButton:hover { background-color: #da190b; }"
    );
    
    pauseResumeButton->setEnabled(true);
    pauseResumeButton->setText("▶️ Resume");
    
    addStreamerButton->setEnabled(false);
    streamerList->setEnabled(false);
  }
}


void MainWindow::OnStartStopClicked() {
  MonitorState state = controller->GetState();
  
  if (state == MonitorState::STOPPED) {
    if (controller->GetAllStreamers().empty()) {
      QMessageBox::warning(this, "No Streamers",
        "Please add at least one streamer before starting.");
      return;
    }
    
    controller->Start();
    
  } else {
    controller->Stop();
  }
}


void MainWindow::OnPauseResumeClicked() {
  MonitorState state = controller->GetState();
  
  if (state == MonitorState::RUNNING) {
    controller->Pause();
    trayIcon->UpdatePauseAction(true);
    
  } else if (state == MonitorState::PAUSED) {
    controller->Resume();
    trayIcon->UpdatePauseAction(false);
  }
}


void MainWindow::OnAddStreamerClicked() {
  int maxPriority = controller->GetAllStreamers().size();
  
  AddStreamerDialog dialog(maxPriority, this);
  
  if (dialog.exec() == QDialog::Accepted) {
    QString name = dialog.GetStreamerName();
    int priority = dialog.GetPriority();
    
    if (controller->AddStreamer(name.toStdString(), priority)) {
      auto streamers = controller->GetAllStreamers();
      streamerList->UpdateStreamers(streamers);
      
      QMessageBox::information(this, "Success",
        QString("Added streamer: %1 (priority: %2)").arg(name).arg(priority));
    } else {
      QMessageBox::warning(this, "Error",
        QString("Failed to add streamer: %1\n"
                "Streamer may already exist or name is invalid.").arg(name));
    }
  }
}


void MainWindow::OnRemoveStreamerClicked() {
  QString name = streamerList->GetSelectedStreamerName();
  
  if (name.isEmpty()) {
    QMessageBox::warning(this, "No Selection",
      "Please select a streamer to remove.");
    return;
  }
  
  auto reply = QMessageBox::question(this, "Confirm Removal",
    QString("Remove streamer: %1?").arg(name),
    QMessageBox::Yes | QMessageBox::No);
  
  if (reply == QMessageBox::Yes) {
    if (controller->RemoveStreamer(name.toStdString())) {
      auto streamers = controller->GetAllStreamers();
      streamerList->UpdateStreamers(streamers);
      
      statusBar()->showMessage(QString("Removed: %1").arg(name), 3000);
    }
  }
}


void MainWindow::OnMoveUpClicked() {
  QString name = streamerList->GetSelectedStreamerName();
  
  if (name.isEmpty()) {
    return;
  }
  
  if (controller->MoveStreamerUp(name.toStdString())) {
    auto streamers = controller->GetAllStreamers();
    streamerList->UpdateStreamers(streamers);
    
    statusBar()->showMessage(QString("Moved up: %1").arg(name), 2000);
  }
}


void MainWindow::OnMoveDownClicked() {
  QString name = streamerList->GetSelectedStreamerName();
  
  if (name.isEmpty()) {
    return;
  }
  
  if (controller->MoveStreamerDown(name.toStdString())) {
    auto streamers = controller->GetAllStreamers();
    streamerList->UpdateStreamers(streamers);
    
    statusBar()->showMessage(QString("Moved down: %1").arg(name), 2000);
  }
}


void MainWindow::OnStreamerStatusChanged(QString name, StreamStatus status) {
  auto streamers = controller->GetAllStreamers();
  
  for (const auto& streamer : streamers) {
    if (QString::fromStdString(streamer.GetName()) == name) {
      streamerList->UpdateStreamerStatus(name, status, streamer.IsTabOpened());
      break;
    }
  }
  
  int openTabs = controller->GetOpenTabsCount();
  openTabsLabel->setText(QString("Open tabs: <b>%1/2</b>").arg(openTabs));
}


void MainWindow::OnStreamerTabOpenedChanged(QString name, bool opened) {
  auto streamers = controller->GetAllStreamers();
  
  for (const auto& streamer : streamers) {
    if (QString::fromStdString(streamer.GetName()) == name) {
      streamerList->UpdateStreamerStatus(name, streamer.GetStatus(), opened);
      break;
    }
  }
  
  int openTabs = controller->GetOpenTabsCount();
  openTabsLabel->setText(QString("Open tabs: <b>%1/2</b>").arg(openTabs));
  
  if (opened) {
    trayIcon->ShowNotification("Stream Online", 
      QString("%1 is now streaming!").arg(name));
  }
}


void MainWindow::OnMonitoringStarted() {
  statusLabel->setText("Status: <b style='color: #4CAF50;'>Running</b>");
  UpdateButtonStates();
  
  trayIcon->ShowNotification("Monitoring Started", 
    "Twitch Stream Monitor is now active");
}


void MainWindow::OnMonitoringStopped() {
  statusLabel->setText("Status: <b style='color: #f44336;'>Stopped</b>");
  openTabsLabel->setText("Open tabs: <b>0/2</b>");
  UpdateButtonStates();
  
  trayIcon->ShowNotification("Monitoring Stopped", 
    "Twitch Stream Monitor has been stopped");
}


void MainWindow::OnMonitoringPaused() {
  statusLabel->setText("Status: <b style='color: #FF9800;'>Paused (Check Only)</b>");
  UpdateButtonStates();
  
  trayIcon->ShowNotification("Monitoring Paused", 
    "Checking streams but not opening tabs");
}


void MainWindow::OnMonitoringResumed() {
  statusLabel->setText("Status: <b style='color: #4CAF50;'>Running</b>");
  UpdateButtonStates();
  
  trayIcon->ShowNotification("Monitoring Resumed", 
    "Twitch Stream Monitor is fully active");
}


void MainWindow::OnErrorOccurred(QString errorMessage) {
  QMessageBox::critical(this, "Error", errorMessage);
  statusBar()->showMessage("Error: " + errorMessage, 10000);
}


void MainWindow::closeEvent(QCloseEvent* event) {
  if (controller->IsRunning()) {
    hide();
    trayIcon->ShowNotification("Still Running", 
      "Twitch Monitor is running in background");
    event->ignore();
  } else {
    event->accept();
  }
}


void MainWindow::OnTrayIconActivated(QSystemTrayIcon::ActivationReason reason) {
  if (reason == QSystemTrayIcon::Trigger || 
      reason == QSystemTrayIcon::DoubleClick) {
    ShowWindow();
  }
}


void MainWindow::ShowWindow() {
  show();
  raise();
  activateWindow();
}


void MainWindow::QuitApplication() {
  if (controller->IsRunning()) {
    auto reply = QMessageBox::question(this, "Quit",
      "Monitoring is active. Stop and quit?",
      QMessageBox::Yes | QMessageBox::No);
    
    if (reply == QMessageBox::No) {
      return;
    }
    
    controller->Stop();
  }
  
  qApp->quit();
}