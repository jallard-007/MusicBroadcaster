/**
 * @author Justin Nicolas Allard
 * Header file for music storage class
*/

#ifndef MUSIC_STORAGE_H
#define MUSIC_STORAGE_H

#include <list>
#include <memory>
#include <atomic>
#include <regex>
#include <mutex>
#include <string>

#include "Music.hpp"

#define MAX_SONGS 10

/**
 * Storage class for music objects. Not fully implemented and integrated with room class
*/
class MusicStorageEntry {
public:
  std::atomic<int> sent;
  int fd;
  std::string path;
  std::mutex entryMutex;
  MusicStorageEntry();
  MusicStorageEntry(int, std::string);
};

class MusicStorage {
private:
  static const std::regex tempFileRegEx;
  /**
   * we dont need the music objects to be next to each other, so use a list
  */
  std::list<MusicStorageEntry> songs;

  std::mutex musicStorageMutex;

  MusicStorageEntry *add(const std::string &path, int fd);

public:
  MusicStorage();
  ~MusicStorage();

  /**
   * Removes a music object by its address in memory
  */
  void removeByAddress(const MusicStorageEntry *);

  MusicStorageEntry *getFirstEmptyAndLockEntry();

  MusicStorageEntry *addEmpty();

  MusicStorageEntry *addAndLockEntry();

  MusicStorageEntry *addLocalAndLockEntry();

  [[nodiscard]] MusicStorageEntry *getFront();

  bool hasPreviousBeenSent(MusicStorageEntry *);

  void removeFront();
};


#endif
