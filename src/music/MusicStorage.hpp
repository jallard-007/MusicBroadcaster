/**
 * @author Justin Nicolas Allard
 * Header file for music storage class
*/

#ifndef MUSIC_STORAGE_H
#define MUSIC_STORAGE_H

#include <list>
#include <memory>
#include <string>

#include "Music.hpp"

#define MAX_SONGS 10

/**
 * Storage class for music objects. Not fully implemented and integrated with room class
*/
class MusicStorageEntry {
public:
  std::string path;
  int fd;
  MusicStorageEntry();
  MusicStorageEntry(std::string, int);
};

class MusicStorage {
private:

  /**
   * we dont need the music objects to be next to each other, so use a list
  */
  std::list<MusicStorageEntry> songs;

public:
  MusicStorage();
  ~MusicStorage();

  /**
   * Removes a music object by its address in memory
  */
  void removeByAddress(const MusicStorageEntry *);

  const MusicStorageEntry *add();

  [[nodiscard]] const MusicStorageEntry *getFront();

  void removeFront();
};

#endif
