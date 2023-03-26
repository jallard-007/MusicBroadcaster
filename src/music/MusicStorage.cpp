/**
 * @author Justin Nicolas Allard
 * Implementation file for music storage class
*/

#include <string>
#include "Music.hpp"
#include "MusicStorage.hpp"

MusicStorage::MusicStorage(): songs{} {}

void MusicStorage::removeByAddress(FILE *musicAddress) {
  songs.remove_if([musicAddress](FILE *song){
    return song == musicAddress;
  });
}

FILE *MusicStorage::add() {
  if (songs.size() >= MAX_SONGS) {
    // no more room in queue
    return nullptr;
  }
  songs.emplace_back(tmpfile());
  return songs.back();
}

FILE *MusicStorage::getFront() {
  if (songs.empty()) {
    return nullptr;
  }
  return songs.front();
}

std::shared_ptr<Music> MusicStorage::getFrontMem() {
  if (songs.empty()) {
    return nullptr;
  }
  auto music = std::make_shared<Music>();
  music.get()->readFileAtPtr(songs.front());
  return music;
}

void MusicStorage::removeFront() {
  if (songs.empty()) {
    return;
  }
  songs.pop_front();
}
