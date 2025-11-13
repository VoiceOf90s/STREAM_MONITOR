#include "StreamerListWidget.h"
#include "StreamerItem.h"
#include <QDropEvent>
#include <QDrag>
#include <QMimeData>
#include <QDebug>


StreamerListWidget::StreamerListWidget(QWidget* parent)
  : QListWidget(parent) {
  
  SetupDragAndDrop();
  
  setStyleSheet(
    "QListWidget {"
    "  background-color: #2b2b2b;"
    "  border: 1px solid #555;"
    "  border-radius: 5px;"
    "  padding: 5px;"
    "}"
    "QListWidget::item {"
    "  padding: 8px;"
    "  margin: 2px;"
    "  border-radius: 3px;"
    "}"
    "QListWidget::item:selected {"
    "  background-color: #4a4a4a;"
    "  border: 1px solid #6a6a6a;"
    "}"
    "QListWidget::item:hover {"
    "  background-color: #3a3a3a;"
    "}"
  );
  
  setMinimumHeight(300);
  setAlternatingRowColors(false);
  setSpacing(2);
}


void StreamerListWidget::SetupDragAndDrop() {
  setDragEnabled(true);
  setAcceptDrops(true);
  setDropIndicatorShown(true);
  setDragDropMode(QAbstractItemView::InternalMove);
  setSelectionMode(QAbstractItemView::SingleSelection);
  setDefaultDropAction(Qt::MoveAction);
}


void StreamerListWidget::UpdateStreamers(const std::vector<StreamerPriority>& streamers) {
  clear();
  
  int priority = 0;
  for (const auto& streamer : streamers) {
    QListWidgetItem* listItem = new QListWidgetItem(this);
    
    StreamerItem* widget = new StreamerItem(
      streamer.GetName(),
      priority,
      streamer.GetStatus(),
      streamer.IsTabOpened(),
      this
    );
    
    listItem->setSizeHint(widget->sizeHint());
    setItemWidget(listItem, widget);
    
    priority++;
  }
  
  qDebug() << "[StreamerListWidget] Updated with" << streamers.size() << "streamers";
}


QString StreamerListWidget::GetSelectedStreamerName() const {
  QListWidgetItem* item = currentItem();
  if (!item) {
    return QString();
  }
  
  StreamerItem* widget = qobject_cast<StreamerItem*>(itemWidget(item));
  if (widget) {
    return QString::fromStdString(widget->GetName());
  }
  
  return QString();
}


int StreamerListWidget::GetSelectedRow() const {
  return currentRow();
}


void StreamerListWidget::UpdateStreamerStatus(const QString& name, 
                                              StreamStatus status, 
                                              bool tabOpened) {
  for (int i = 0; i < count(); i++) {
    QListWidgetItem* listItem = item(i);
    StreamerItem* widget = qobject_cast<StreamerItem*>(itemWidget(listItem));
    
    if (widget && QString::fromStdString(widget->GetName()) == name) {
      widget->UpdateStatus(status, tabOpened);
      break;
    }
  }
}


void StreamerListWidget::dropEvent(QDropEvent* event) {
  int oldRow = currentRow();
  
  QListWidget::dropEvent(event);
  
  int newRow = currentRow();
  
  if (oldRow != newRow && oldRow >= 0 && newRow >= 0) {
    qDebug() << "[StreamerListWidget] Item moved from" << oldRow << "to" << newRow;
    emit streamerOrderChanged();
  }
}