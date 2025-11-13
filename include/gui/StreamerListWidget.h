#ifndef STREAMER_LIST_WIDGET_H
#define STREAMER_LIST_WIDGET_H

#include <QListWidget>
#include <QListWidgetItem>
#include <vector>
#include "StreamerPriority.h"


// Виджет для отображения списка стримеров с drag-and-drop
class StreamerListWidget : public QListWidget {
  Q_OBJECT

private:
  void SetupDragAndDrop();


protected:
  void dropEvent(QDropEvent* event) override;


public:
  explicit StreamerListWidget(QWidget* parent = nullptr);
  
  // Обновление списка
  void UpdateStreamers(const std::vector<StreamerPriority>& streamers);
  
  // Получить выбранного стримера
  QString GetSelectedStreamerName() const;
  int GetSelectedRow() const;
  
  // Обновление статуса конкретного стримера
  void UpdateStreamerStatus(const QString& name, StreamStatus status, bool tabOpened);


signals:
  void streamerOrderChanged();
  void removeRequested();
  void moveUpRequested();
  void moveDownRequested();
};

#endif // STREAMER_LIST_WIDGET_H