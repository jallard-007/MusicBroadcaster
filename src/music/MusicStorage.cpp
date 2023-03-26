/**
 * @author Justin Nicolas Allard
 * Implementation file for music storage class
*/

#include <string>
#include "Music.hpp"
#include "MusicStorage.hpp"

MusicStorage::MusicStorage(): songs{} {
}

void MusicStorage::removeByPath(const std::string &musicPath) {
  songs.remove_if([musicPath](Music &song){
    return song.getPath() == musicPath;
  });
}

void MusicStorage::removeByName(const std::string &musicName) {
  songs.remove_if([musicName](Music &song){
    return song.getName() == musicName;
  });
}
void MusicStorage::removeByAddress(Music *musicAddress) {
  songs.remove_if([musicAddress](Music &song){
    return &song == musicAddress;
  });
}

Music *MusicStorage::add() {
  if (songs.size() >= MAX_SONGS) {
    // no more room in queue
    return nullptr;
  }
  songs.emplace_back("", "");
  return &songs.back();
}

Music *MusicStorage::addByName(const std::string &musicName) {
  if (songs.size() >= MAX_SONGS) {
    // no more room in queue
    return nullptr;
  }
  songs.emplace_back(musicName, "");
  return &songs.back();
}

Music *MusicStorage::getByPath(const std::string &musicPath) {
  for (Music &song : songs) {
    if (song.getPath() == musicPath) {
      return &song;
    }
  }
  return nullptr;
}

Music *MusicStorage::getByName(const std::string &musicName) {
  for (Music &song : songs) {
    if (song.getName() == musicName) {
      return &song;
    }
  }
  return nullptr;
}

Music *MusicStorage::getFront() {
  if (songs.empty()) {
    return nullptr;
  }
  return &songs.front();
}

void MusicStorage::removeFront() {
  if (songs.empty()) {
    return;
  }
  songs.pop_front();
}
