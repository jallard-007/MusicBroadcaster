/**
 * @author Justin Nicolas Allard
 * Implementation file for music storage class
*/

#include <string>
#include <iostream>
#if _WIN32
// windows includes
#elif defined(__APPLE__) || defined(__unix__)
#include <unistd.h>
#endif

#include "../debug.hpp"
#include "MusicStorage.hpp"

MusicStorageEntry::MusicStorageEntry():
  sent{false}, fd{0}, path{},  entryMutex{} {}

MusicStorageEntry::MusicStorageEntry(int i, std::string s):
  sent{false}, fd{i}, path{std::move(s)},  entryMutex{} {}

const std::regex MusicStorage::tempFileRegEx{"/tmp/musicBroadcaster_[-a-zA-Z0-9._]{6}"};

MusicStorage::MusicStorage(): songs{}, musicStorageMutex{} {}

MusicStorage::~MusicStorage() {
  for (MusicStorageEntry &entry: songs) {
    // double guard against deleting local files
    if (entry.fd > 0 && std::regex_match(songs.front().path, tempFileRegEx)) {
      remove(entry.path.c_str());
    }
  }
}

MusicStorageEntry *MusicStorage::add(const std::string &path, int fd) {
  DEBUG_P(std::cout << "waiting for queue mutex\n");
  musicStorageMutex.lock();
  DEBUG_P(std::cout << "got queue mutex\n");
  auto &p_entry = songs.emplace_back(fd, path);
  p_entry.entryMutex.lock();
  DEBUG_P(std::cout << "got entry mutex\n");
  musicStorageMutex.unlock();
  DEBUG_P(std::cout << "unlocked queue mutex\n");
  return &p_entry;
}

MusicStorageEntry *MusicStorage::addAtIndexAndLock(uint8_t index) {
  DEBUG_P(std::cout << "adding at index [" << (int)index << "]\n");
  if (index > MAX_SONGS - 1) {
    std::cerr << "Request to add song at index past max songs (" << MAX_SONGS << ")\n";
    exit(1);
  }

  if (songs.empty() || songs.size() - 1 < index) {
    DEBUG_P(std::cout << "need to add entries to reach index \n");
    DEBUG_P(std::cout << "waiting for queue mutex\n");
    std::unique_lock<std::mutex> lock{musicStorageMutex};
    DEBUG_P(std::cout << "got queue mutex\n");
    do {
      songs.emplace_back(0, "");
    } while (songs.size() - 1 < index);
    auto &back = songs.back();
    back.entryMutex.lock();
    DEBUG_P(std::cout << "got entry mutex\n");
    DEBUG_P(std::cout << "unlocked queue mutex\n");
    return &back;
  }
  DEBUG_P(std::cout << "no need to add entries\n");
  auto iter = songs.begin();
  for (int i = 0; i < index; ++i, ++iter);
  if (!iter->entryMutex.try_lock()) {
    std::cerr << "could not get entry lock, there's an issue with the code\n";
    exit(1);
  }
  DEBUG_P(std::cout << "got entry mutex\n");
  return &*iter;
}

bool MusicStorage::makeTemp(MusicStorageEntry *p_entry) {
  if (p_entry == nullptr) {
    return false;
  }
  char s[] = "/tmp/musicBroadcaster_XXXXXX";
  int filedes = mkstemp(s);
  if (filedes < 1) {
    return false;
  }
  p_entry->path = s;
  p_entry->fd = filedes;
  return true;
}

MusicStorageEntry *MusicStorage::addLocalAndLockEntry() {
  if (songs.size() >= MAX_SONGS) {
    return nullptr;
  }
  return add("path", -1);
}

MusicStorageEntry *MusicStorage::addAndLockEntry() {
  if (songs.size() >= MAX_SONGS) {
    return nullptr;
  }
  char s[] = "/tmp/musicBroadcaster_XXXXXX";
  int filedes = mkstemp(s);
  if (filedes < 1) {
    return nullptr;
  }
  return add(s, filedes);
}

MusicStorageEntry *MusicStorage::getFront() {
  if (songs.empty()) {
    return nullptr;
  }
  return &songs.front();
}

void MusicStorage::removeFront() {
  DEBUG_P(std::cout << "waiting for queue mutex\n");
  musicStorageMutex.lock();
  DEBUG_P(std::cout << "got queue mutex\n");
  // double guard against deleting local files
  DEBUG_P(std::cout << "waiting for entry mutex\n");
  songs.front().entryMutex.lock();
  DEBUG_P(std::cout << "got entry mutex\n");
  if (songs.front().fd > 0 && std::regex_match(songs.front().path, tempFileRegEx)) {
    remove(songs.front().path.c_str());
  }
  songs.pop_front();
  DEBUG_P(std::cout << "deleted front entry\n");
  musicStorageMutex.unlock();
  DEBUG_P(std::cout << "unlocked queue mutex\n");
}

const std::list<MusicStorageEntry> &MusicStorage::getSongs() const {
  return songs;
}

void MusicStorage::removeByAddress(const MusicStorageEntry *musicAddress) {
  if (musicAddress == nullptr) {
    return;
  }
  DEBUG_P(std::cout << "waiting for queue mutex\n");
  std::unique_lock<std::mutex> lock(musicStorageMutex);
  DEBUG_P(std::cout << "got queue mutex\n");
  songs.remove_if([musicAddress](const MusicStorageEntry &song){
    return &song == musicAddress;
  });
  DEBUG_P(std::cout << "unlocked queue mutex\n");
}

void MusicStorage::removeByPosition(uint8_t position) {
  if (position > MAX_SONGS - 1) {
    return;
  }
  DEBUG_P(std::cout << "waiting for queue mutex\n");
  std::unique_lock<std::mutex> lock(musicStorageMutex);
  DEBUG_P(std::cout << "got queue mutex\n");
  uint8_t i = 0;
  auto iter = songs.begin();
  while (iter != songs.end()) {
    if (i == position) {
      songs.erase(iter);
      DEBUG_P(std::cout << "unlocked queue mutex\n");
      return;
    }
    ++i;
    ++iter;
  }
  DEBUG_P(std::cout << "unlocked queue mutex\n");
}

int MusicStorage::getPositionInQueue(const MusicStorageEntry *p_find) const {
  uint8_t i = 0;
  for (const MusicStorageEntry &entry : songs) {
    if (&entry == p_find) {
      return static_cast<int>(i);
    }
    ++i;
  }
  return -1;
}
