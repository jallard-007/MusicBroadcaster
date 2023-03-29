/**
 * @author Justin Nicolas Allard
 * Implementation file for music storage class
*/

#include <string>
#include <iostream>
#include <unistd.h>
#include "Music.hpp"
#include "MusicStorage.hpp"

MusicStorageEntry::MusicStorageEntry():
  path{""}, fd{0} {}

MusicStorageEntry::MusicStorageEntry(std::string s, int i):
  path{std::move(s)}, fd{i} {}

MusicStorage::MusicStorage(): songs{} {}

MusicStorage::~MusicStorage() {
  for (MusicStorageEntry &entry: songs) {
    if (entry.fd != 0) {
      remove(entry.path.c_str());
    }
  }
}

void MusicStorage::removeByAddress(const MusicStorageEntry *musicAddress) {
  songs.remove_if([musicAddress](const MusicStorageEntry &song){
    return &song == musicAddress;
  });
}

const MusicStorageEntry *MusicStorage::add() {
  if (songs.size() >= MAX_SONGS) {
    return nullptr;
  }
  char s[] = "/tmp/musicBroadcaster_XXXXXX";
  int filedes = mkstemp(s);
  if (filedes < 1) {
    return nullptr;
  }
  return &songs.emplace_back(s, filedes);
}

const MusicStorageEntry *MusicStorage::getFront() {
  if (songs.size() == 0) {
    return nullptr;
  }
  return &songs.front();
}

void MusicStorage::removeFront() {
  if (songs.front().fd != 0) {
    close(songs.front().fd);
    remove(songs.front().path.c_str());
  }
  songs.pop_front();
}
