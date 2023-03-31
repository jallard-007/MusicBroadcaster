/**
 * @author Justin Nicolas Allard
 * Implementation file for music storage class
*/

#include <string>
#include <iostream>
#include <unistd.h>

#include "../debug.hpp"
#include "Music.hpp"
#include "MusicStorage.hpp"

MusicStorageEntry::MusicStorageEntry():
  sent{false}, fd{0}, path{""},  entryMutex{} {}

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

void MusicStorage::removeByAddress(const MusicStorageEntry *musicAddress) {
  DEBUG_P(std::cout << "waiting for queue mutex\n");
  musicStorageMutex.lock();
  DEBUG_P(std::cout << "got queue mutex\n");
  songs.remove_if([musicAddress](const MusicStorageEntry &song){
    return &song == musicAddress;
  });
  musicStorageMutex.unlock();
  DEBUG_P(std::cout << "unlocked queue mutex\n");
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

MusicStorageEntry *MusicStorage::getFirstEmptyAndLockEntry() {
  if (songs.size() >= MAX_SONGS) {
    return nullptr;
  }
  char s[] = "/tmp/musicBroadcaster_XXXXXX";
  int filedes = mkstemp(s);
  if (filedes < 1) {
    return nullptr;
  }
  DEBUG_P(std::cout << "waiting for queue mutex\n");
  musicStorageMutex.lock();
  DEBUG_P(std::cout << "got queue mutex\n");
  for (MusicStorageEntry &entry : songs) {
    if (entry.path == "" && entry.entryMutex.try_lock()) {
      DEBUG_P(std::cout << "got entry mutex\n");
      entry.path = s;
      entry.fd = filedes;
      musicStorageMutex.unlock();
      DEBUG_P(std::cout << "unlocked queue mutex\n");
      return &entry;
    }
  }
  auto &p_entry = songs.emplace_back(filedes, s);
  p_entry.entryMutex.lock();
  DEBUG_P(std::cout << "got entry mutex\n");
  musicStorageMutex.unlock();
  DEBUG_P(std::cout << "unlocked queue mutex\n");
  return &p_entry;
}

MusicStorageEntry *MusicStorage::addEmpty() {
  DEBUG_P(std::cout << "waiting for queue mutex\n");
  musicStorageMutex.lock();
  DEBUG_P(std::cout << "got queue mutex\n");
  auto &p_entry = songs.emplace_back(0, "");
  musicStorageMutex.unlock();
  DEBUG_P(std::cout << "unlocked queue mutex\n");
  return &p_entry;
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

MusicStorageEntry *MusicStorage::addLocalAndLockEntry() {
  if (songs.size() >= MAX_SONGS) {
    return nullptr;
  }
  return add("path", -1);
}

MusicStorageEntry *MusicStorage::getFront() {
  if (songs.size() == 0) {
    return nullptr;
  }
  return &songs.front();
}

bool MusicStorage::hasPreviousBeenSent(MusicStorageEntry *curr) {
  auto iter = songs.begin();
  while (iter != songs.end()) {
    if (&*iter == curr) {
      if (&songs.front() != curr) {
        return (--iter)->sent > 0;
      } else {
        return true;
      }
    }
    ++iter;
  }
  return true;
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
  musicStorageMutex.unlock();
  DEBUG_P(std::cout << "unlocked queue mutex\n");
}
