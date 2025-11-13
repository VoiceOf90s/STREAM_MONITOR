#include "StreamerItem.h"
#include <QFont>


StreamerItem::StreamerItem(const std::string& name, int priorityIndex, 
                          StreamStatus status, bool tabOpen, 
                          QWidget* parent)
  : QWidget(parent), priority(priorityIndex), streamerName(name),
    currentStatus(status), tabOpened(tabOpen) {
  
  SetupUI();
  UpdateStatusIcon();
  UpdateTabIcon();
}


void StreamerItem::SetupUI() {
  QHBoxLayout* layout = new QHBoxLayout(this);
  layout->setContentsMargins(8, 4, 8, 4);
  layout->setSpacing(10);
  
  priorityLabel = new QLabel(QString::number(priority), this);
  priorityLabel->setStyleSheet(
    "QLabel {"
    "  background-color: #555;"
    "  color: white;"
    "  font-weight: bold;"
    "  border-radius: 10px;"
    "  min-width: 20px;"
    "  max-width: 20px;"
    "  min-height: 20px;"
    "  max-height: 20px;"
    "  padding: 2px;"
    "}"
  );
  priorityLabel->setAlignment(Qt::AlignCenter);
  
  nameLabel = new QLabel(QString::fromStdString(streamerName), this);
  QFont nameFont = nameLabel->font();
  nameFont.setPointSize(11);
  nameFont.setBold(true);
  nameLabel->setFont(nameFont);
  nameLabel->setStyleSheet("QLabel { color: white; }");
  
  statusIcon = new QLabel(this);
  statusIcon->setFixedSize(16, 16);
  statusIcon->setScaledContents(true);
  
  statusText = new QLabel(this);
  QFont statusFont = statusText->font();
  statusFont.setPointSize(9);
  statusText->setFont(statusFont);
  
  tabIcon = new QLabel(this);
  tabIcon->setFixedSize(16, 16);
  tabIcon->setScaledContents(true);
  
  layout->addWidget(priorityLabel);
  layout->addWidget(nameLabel);
  layout->addStretch();
  layout->addWidget(statusIcon);
  layout->addWidget(statusText);
  layout->addWidget(tabIcon);
  
  setLayout(layout);
  setMinimumHeight(40);
}


void StreamerItem::UpdateStatusIcon() {
  QString iconText;
  QString textColor;
  QString statusStr;
  
  switch (currentStatus) {
    case StreamStatus::UNKNOWN:
      iconText = "❓";
      textColor = "#888";
      statusStr = "Unknown";
      break;
      
    case StreamStatus::CHECKING:
      iconText = "🔄";
      textColor = "#FFD700";
      statusStr = "Checking...";
      break;
      
    case StreamStatus::ONLINE:
      iconText = "🟢";
      textColor = "#4CAF50";
      statusStr = "Online";
      break;
      
    case StreamStatus::OFFLINE:
      iconText = "⚫";
      textColor = "#f44336";
      statusStr = "Offline";
      break;
  }
  
  statusIcon->setText(iconText);
  statusText->setText(statusStr);
  statusText->setStyleSheet(QString("QLabel { color: %1; font-weight: bold; }").arg(textColor));
}


void StreamerItem::UpdateTabIcon() {
  if (tabOpened) {
    tabIcon->setText("📺");
    tabIcon->setToolTip("Browser tab opened");
  } else {
    tabIcon->setText("");
    tabIcon->setToolTip("");
  }
}


void StreamerItem::UpdateStatus(StreamStatus status, bool tabOpen) {
  currentStatus = status;
  tabOpened = tabOpen;
  
  UpdateStatusIcon();
  UpdateTabIcon();
}


void StreamerItem::UpdatePriority(int newPriority) {
  priority = newPriority;
  priorityLabel->setText(QString::number(priority));
}