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

// ABSOLUTE max is the max number representable by the size of 'option' in Message class, so currently 255 (1 byte)
#define MAX_SONGS 50

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


  MusicStorageEntry *addAtIndexAndLock(uint8_t index);

  static bool makeTemp(MusicStorageEntry *);

  MusicStorageEntry *addAndLockEntry();

  MusicStorageEntry *addLocalAndLockEntry();

  int getPositionInQueue(const MusicStorageEntry *) const;

  [[nodiscard]] MusicStorageEntry *getFront();

  [[nodiscard]] const std::list<MusicStorageEntry> &getSongs() const;

  void removeFront();

  /**
   * Removes a music object by its address in memory
  */
  void removeByAddress(const MusicStorageEntry *);
};


#endif
