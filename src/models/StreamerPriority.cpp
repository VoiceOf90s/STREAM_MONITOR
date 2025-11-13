#include "StreamerPriority.h"
#include "StringUtils.h"
#include <algorithm>
#include <fstream>
#include <iostream>
#include <chrono>


// ==================== StreamerPriority Implementation ====================

StreamerPriority::StreamerPriority(const std::string& streamerName, int priority)
  : name(streamerName), priorityIndex(priority), status(StreamStatus::UNKNOWN),
    tabOpened(false), lastCheckTime(0), onlineDetectedTime(0),
    dataMutex(std::make_shared<std::mutex>()) {
}


std::string StreamerPriority::GetName() const {
  std::lock_guard<std::mutex> lock(*dataMutex);
  return name;
}


int StreamerPriority::GetPriority() const {
  std::lock_guard<std::mutex> lock(*dataMutex);
  return priorityIndex;
}


StreamStatus StreamerPriority::GetStatus() const {
  std::lock_guard<std::mutex> lock(*dataMutex);
  return status;
}


bool StreamerPriority::IsTabOpened() const {
  std::lock_guard<std::mutex> lock(*dataMutex);
  return tabOpened;
}


long long StreamerPriority::GetLastCheckTime() const {
  std::lock_guard<std::mutex> lock(*dataMutex);
  return lastCheckTime;
}


long long StreamerPriority::GetOnlineDetectedTime() const {
  std::lock_guard<std::mutex> lock(*dataMutex);
  return onlineDetectedTime;
}


void StreamerPriority::SetPriority(int priority) {
  std::lock_guard<std::mutex> lock(*dataMutex);
  priorityIndex = priority;
}


void StreamerPriority::SetStatus(StreamStatus newStatus) {
  std::lock_guard<std::mutex> lock(*dataMutex);
  status = newStatus;
}


void StreamerPriority::SetTabOpened(bool opened) {
  std::lock_guard<std::mutex> lock(*dataMutex);
  tabOpened = opened;
}


void StreamerPriority::UpdateLastCheckTime() {
  std::lock_guard<std::mutex> lock(*dataMutex);
  lastCheckTime = std::chrono::duration_cast<std::chrono::seconds>(
    std::chrono::system_clock::now().time_since_epoch()
  ).count();
}


void StreamerPriority::SetOnlineDetectedTime(long long timestamp) {
  std::lock_guard<std::mutex> lock(*dataMutex);
  onlineDetectedTime = timestamp;
}


bool StreamerPriority::ShouldCheck(int maxPriorityToCheck) const {
  std::lock_guard<std::mutex> lock(*dataMutex);
  return priorityIndex <= maxPriorityToCheck;
}


bool StreamerPriority::operator<(const StreamerPriority& other) const {
  return priorityIndex < other.priorityIndex;
}


// ==================== StreamerPriorityManager Implementation ====================

StreamerPriorityManager::StreamerPriorityManager(const std::string& file)
  : filePath(file) {
  LoadFromFile();
}


void StreamerPriorityManager::SortByPriority() {
  std::sort(streamers.begin(), streamers.end());
}


bool StreamerPriorityManager::LoadFromFile() {
  std::lock_guard<std::mutex> lock(managerMutex);
  
  std::ifstream file(filePath);
  if (!file.is_open()) {
    std::cerr << "[StreamerPriorityManager] Cannot open file: " << filePath << std::endl;
    return false;
  }
  
  streamers.clear();
  std::string line;
  int priority = 0;
  
  while (std::getline(file, line)) {
    line = StringUtils::Trim(line);
    
    if (line.empty() || line[0] == '#') {
      continue;
    }
    
    if (StringUtils::IsValidStreamerName(line)) {
      streamers.emplace_back(line, priority);
      priority++;
    }
  }
  
  file.close();
  
  std::cout << "[StreamerPriorityManager] Loaded " << streamers.size() 
            << " streamers from " << filePath << std::endl;
  
  return true;
}


bool StreamerPriorityManager::SaveToFile() {
  std::lock_guard<std::mutex> lock(managerMutex);
  
  std::ofstream file(filePath);
  if (!file.is_open()) {
    std::cerr << "[StreamerPriorityManager] Cannot save to file: " << filePath << std::endl;
    return false;
  }
  
  file << "# Twitch Streamers List (Prioritized)" << std::endl;
  file << "# Priority: 0 = highest, " << (streamers.size() - 1) << " = lowest" << std::endl;
  file << "# Lines starting with # are comments" << std::endl;
  file << std::endl;
  
  for (const auto& streamer : streamers) {
    file << streamer.GetName() << std::endl;
  }
  
  file.close();
  
  std::cout << "[StreamerPriorityManager] Saved " << streamers.size() 
            << " streamers to " << filePath << std::endl;
  
  return true;
}


bool StreamerPriorityManager::AddStreamer(const std::string& name, int priority) {
  std::lock_guard<std::mutex> lock(managerMutex);
  
  if (!StringUtils::IsValidStreamerName(name)) {
    std::cerr << "[StreamerPriorityManager] Invalid streamer name: " << name << std::endl;
    return false;
  }
  
  for (const auto& streamer : streamers) {
    if (streamer.GetName() == name) {
      std::cerr << "[StreamerPriorityManager] Streamer already exists: " << name << std::endl;
      return false;
    }
  }
  
  int finalPriority = (priority < 0) ? static_cast<int>(streamers.size()) : priority;
  streamers.emplace_back(name, finalPriority);
  
  SortByPriority();
  SaveToFile();
  
  return true;
}


bool StreamerPriorityManager::RemoveStreamer(const std::string& name) {
  std::lock_guard<std::mutex> lock(managerMutex);
  
  auto it = std::find_if(streamers.begin(), streamers.end(),
    [&name](const StreamerPriority& s) { return s.GetName() == name; });
  
  if (it == streamers.end()) {
    return false;
  }
  
  streamers.erase(it);
  
  for (size_t i = 0; i < streamers.size(); i++) {
    streamers[i].SetPriority(static_cast<int>(i));
  }
  
  SaveToFile();
  return true;
}


bool StreamerPriorityManager::MoveUp(const std::string& name) {
  std::lock_guard<std::mutex> lock(managerMutex);
  
  auto it = std::find_if(streamers.begin(), streamers.end(),
    [&name](const StreamerPriority& s) { return s.GetName() == name; });
  
  if (it == streamers.end() || it == streamers.begin()) {
    return false;
  }
  
  std::iter_swap(it, it - 1);
  
  for (size_t i = 0; i < streamers.size(); i++) {
    streamers[i].SetPriority(static_cast<int>(i));
  }
  
  SaveToFile();
  return true;
}


bool StreamerPriorityManager::MoveDown(const std::string& name) {
  std::lock_guard<std::mutex> lock(managerMutex);
  
  auto it = std::find_if(streamers.begin(), streamers.end(),
    [&name](const StreamerPriority& s) { return s.GetName() == name; });
  
  if (it == streamers.end() || it == streamers.end() - 1) {
    return false;
  }
  
  std::iter_swap(it, it + 1);
  
  for (size_t i = 0; i < streamers.size(); i++) {
    streamers[i].SetPriority(static_cast<int>(i));
  }
  
  SaveToFile();
  return true;
}


void StreamerPriorityManager::SetPriority(const std::string& name, int newPriority) {
  std::lock_guard<std::mutex> lock(managerMutex);
  
  auto it = std::find_if(streamers.begin(), streamers.end(),
    [&name](const StreamerPriority& s) { return s.GetName() == name; });
  
  if (it != streamers.end()) {
    it->SetPriority(newPriority);
    SortByPriority();
    SaveToFile();
  }
}


std::vector<StreamerPriority> StreamerPriorityManager::GetAllStreamers() const {
  std::lock_guard<std::mutex> lock(managerMutex);
  return streamers;
}


StreamerPriority* StreamerPriorityManager::GetStreamer(const std::string& name) {
  std::lock_guard<std::mutex> lock(managerMutex);
  
  auto it = std::find_if(streamers.begin(), streamers.end(),
    [&name](const StreamerPriority& s) { return s.GetName() == name; });
  
  return (it != streamers.end()) ? &(*it) : nullptr;
}


int StreamerPriorityManager::GetStreamerCount() const {
  std::lock_guard<std::mutex> lock(managerMutex);
  return static_cast<int>(streamers.size());
}


void StreamerPriorityManager::UpdateStatus(const std::string& name, StreamStatus status) {
  std::lock_guard<std::mutex> lock(managerMutex);
  
  auto it = std::find_if(streamers.begin(), streamers.end(),
    [&name](const StreamerPriority& s) { return s.GetName() == name; });
  
  if (it != streamers.end()) {
    it->SetStatus(status);
  }
}


void StreamerPriorityManager::UpdateTabOpened(const std::string& name, bool opened) {
  std::lock_guard<std::mutex> lock(managerMutex);
  
  auto it = std::find_if(streamers.begin(), streamers.end(),
    [&name](const StreamerPriority& s) { return s.GetName() == name; });
  
  if (it != streamers.end()) {
    it->SetTabOpened(opened);
  }
}


std::vector<std::string> StreamerPriorityManager::GetStreamersToCheck() const {
  std::lock_guard<std::mutex> lock(managerMutex);
  
  std::vector<std::string> result;
  
  std::vector<int> onlinePriorities;
  for (const auto& streamer : streamers) {
    if (streamer.GetStatus() == StreamStatus::ONLINE) {
      onlinePriorities.push_back(streamer.GetPriority());
    }
  }
  
  int maxPriorityToCheck = -1;
  if (onlinePriorities.size() >= 2) {
    std::sort(onlinePriorities.begin(), onlinePriorities.end());
    maxPriorityToCheck = onlinePriorities[1];
  } else {
    maxPriorityToCheck = 999999;
  }
  
  for (const auto& streamer : streamers) {
    if (streamer.ShouldCheck(maxPriorityToCheck)) {
      result.push_back(streamer.GetName());
    }
  }
  
  return result;
}


std::vector<std::string> StreamerPriorityManager::GetOnlineStreamers() const {
  std::lock_guard<std::mutex> lock(managerMutex);
  
  std::vector<std::string> result;
  for (const auto& streamer : streamers) {
    if (streamer.GetStatus() == StreamStatus::ONLINE) {
      result.push_back(streamer.GetName());
    }
  }
  
  return result;
}


int StreamerPriorityManager::GetOpenTabsCount() const {
  std::lock_guard<std::mutex> lock(managerMutex);
  
  int count = 0;
  for (const auto& streamer : streamers) {
    if (streamer.IsTabOpened()) {
      count++;
    }
  }
  
  return count;
}