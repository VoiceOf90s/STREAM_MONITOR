#ifndef STREAMER_ITEM_H
#define STREAMER_ITEM_H

#include <QWidget>
#include <QLabel>
#include <QHBoxLayout>
#include <QPixmap>
#include "StreamerPriority.h"


// Кастомный виджет для отображения одного стримера
class StreamerItem : public QWidget {
  Q_OBJECT

private:
  QLabel* priorityLabel;
  QLabel* nameLabel;
  QLabel* statusIcon;
  QLabel* statusText;
  QLabel* tabIcon;
  
  int priority;
  std::string streamerName;
  StreamStatus currentStatus;
  bool tabOpened;
  
  void SetupUI();
  void UpdateStatusIcon();
  void UpdateTabIcon();


public:
  explicit StreamerItem(const std::string& name, int priorityIndex, 
                       StreamStatus status, bool tabOpen, 
                       QWidget* parent = nullptr);
  
  void UpdateStatus(StreamStatus status, bool tabOpen);
  void UpdatePriority(int newPriority);
  
  std::string GetName() const { return streamerName; }
  int GetPriority() const { return priority; }
};

#endif // STREAMER_ITEM_H