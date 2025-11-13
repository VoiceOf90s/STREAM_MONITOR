#ifndef STREAMER_PRIORITY_H
#define STREAMER_PRIORITY_H

#include <string>
#include <vector>
#include <mutex>
#include <memory>


enum class StreamStatus {
  UNKNOWN,
  CHECKING,
  ONLINE,
  OFFLINE
};


// Класс для хранения данных о стримере с приоритетом
class StreamerPriority {
private:
  std::string name;
  int priorityIndex;  // Чем меньше - тем важнее (0 = самый важный)
  StreamStatus status;
  bool tabOpened;
  long long lastCheckTime;
  long long onlineDetectedTime;  // Когда стрим пошел онлайн
  
  mutable std::shared_ptr<std::mutex> dataMutex;


public:
  StreamerPriority(const std::string& streamerName, int priority);
  
  // Getters (thread-safe)
  std::string GetName() const;
  int GetPriority() const;
  StreamStatus GetStatus() const;
  bool IsTabOpened() const;
  long long GetLastCheckTime() const;
  long long GetOnlineDetectedTime() const;
  
  // Setters (thread-safe)
  void SetPriority(int priority);
  void SetStatus(StreamStatus newStatus);
  void SetTabOpened(bool opened);
  void UpdateLastCheckTime();
  void SetOnlineDetectedTime(long long timestamp);
  
  // Проверка, стоит ли проверять статус этого стримера
  bool ShouldCheck(int maxPriorityToCheck) const;
  
  // Для сортировки по приоритету
  bool operator<(const StreamerPriority& other) const;
};


// Менеджер списка стримеров с приоритетами
class StreamerPriorityManager {
private:
  std::vector<StreamerPriority> streamers;
  mutable std::mutex managerMutex;
  std::string filePath;
  
  void SortByPriority();


public:
  StreamerPriorityManager(const std::string& file = "streamers.txt");
  
  // Загрузка/сохранение из файла
  bool LoadFromFile();
  bool SaveToFile();
  
  // Управление списком (thread-safe)
  bool AddStreamer(const std::string& name, int priority = -1);
  bool RemoveStreamer(const std::string& name);
  bool MoveUp(const std::string& name);
  bool MoveDown(const std::string& name);
  void SetPriority(const std::string& name, int newPriority);
  
  // Получение данных
  std::vector<StreamerPriority> GetAllStreamers() const;
  StreamerPriority* GetStreamer(const std::string& name);
  int GetStreamerCount() const;
  
  // Обновление статусов (thread-safe)
  void UpdateStatus(const std::string& name, StreamStatus status);
  void UpdateTabOpened(const std::string& name, bool opened);
  
  // Получить список стримеров для проверки
  // (не проверяем стримеров ниже 2-го онлайн)
  std::vector<std::string> GetStreamersToCheck() const;
  
  // Получить список онлайн стримеров (отсортированных по приоритету)
  std::vector<std::string> GetOnlineStreamers() const;
  
  // Сколько вкладок сейчас открыто
  int GetOpenTabsCount() const;
};

#endif // STREAMER_PRIORITY_H