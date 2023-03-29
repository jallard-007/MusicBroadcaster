/**
 * @author Justin Nicolas Allard
 * Implementation file for music storage class
*/

#include <string>
#include <iostream>
#include <unistd.h>
#include "Music.hpp"
#include "MusicStorage.hpp"

MusicStorageEntry::MusicStorageEntry(std::string s, int i):
  path{std::move(s)}, fd{i} {}

MusicStorage::MusicStorage(): songs{} {}

void MusicStorage::removeByAddress(const MusicStorageEntry *musicAddress) {
  songs.remove_if([musicAddress](const MusicStorageEntry &song){
    return &song == musicAddress;
  });
}

const MusicStorageEntry *MusicStorage::add() {
  if (songs.size() >= MAX_SONGS) {
    // no more room in queue
    return nullptr;
  }
  char s[] = "/tmp/musicBroadcaster_XXXXXX";
  int filedes = mkstemp(s);
  if (filedes < 1) {
    return nullptr;
  }
  unlink(s);
  songs.emplace_back(s, filedes);
  return &songs.back();
}

const MusicStorageEntry *MusicStorage::getFront() {
  if (songs.empty()) {
    return nullptr;
  }
  return &songs.front();
}

std::shared_ptr<Music> MusicStorage::getFrontMem() {
  if (songs.empty()) {
    return nullptr;
  }
  auto filePath = songs.front();
  auto music = std::make_shared<Music>();
  music.get()->setPath(filePath.path);
  music.get()->readFileAtPath();
  return music;
}

void MusicStorage::removeFront() {
  if (songs.empty()) {
    return;
  }
  close(songs.front().fd);
  songs.pop_front();
}
