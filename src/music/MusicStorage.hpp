/**
 * @author Justin Nicolas Allard
 * @brief This handles the storage of different music objects
 * @version 1.5
 * @date 2023-04-02
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#pragma once

#include <list>
#include <memory>
#include <atomic>
#include <regex>
#include <mutex>
#include <string>
#include <iostream>
#if _WIN32
// windows includes
#elif defined(__APPLE__) || defined(__unix__)
#include <unistd.h>
#endif

#include "../debug.hpp"
#include "Music.hpp"

// ABSOLUTE max is the max number representable by the size of 'option' in Message class, so currently 255 (1 byte)
#define MAX_SONGS 50

/**
 * Entry in the MusicStorage list
*/
class MusicStorageEntry {
public:
  /**
   * If/how many times this entry has been sent
  */
  std::atomic<int> sent;

  /**
   * File descriptor of the file which holds the music information
  */
  int fd;
  /**
   * Path to the file
  */
  std::string path;
  /**
   * mutex for the entry
  */
  std::mutex entryMutex;

  /**
   * Constructor
  */
  MusicStorageEntry();

  /**
   * Constructor
  */
  MusicStorageEntry(int, std::string);
};

class MusicStorage {
private:
  /**
   * Regular expression to match temp file paths
  */
  static const std::regex tempFileRegEx;
  /**
   * we dont need the music objects to be next to each other, so use a list
  */
  std::list<MusicStorageEntry> songs;

  /**
   * Mutex for the list
  */
  std::mutex musicStorageMutex;

  /**
   * @brief Add an unnamed Music object to the back of the list
   * @returns pointer to the new Music object, nullptr if no room
  */
  MusicStorageEntry *add(const std::string &path, int fd);

public:

  /**
   * @brief Construct a new Music Storage object
   * 
   */
  MusicStorage();

  /**
   * @brief Destroy the Music Storage object
   * 
   */
  ~MusicStorage();

  /** 
   * @brief Adds an entry at the the specified index in the queue and locks it's mutex
   * 
   * @return pointer to the entry
   */
  MusicStorageEntry *addAtIndexAndLock(uint8_t index);

  /** 
   * @brief Set an entry's path to a new temp file
   * 
   * @param pointer to the entry
   * @return true on success, false on error
   */
  static bool makeTemp(MusicStorageEntry *);


  /** 
   * @brief Adds an entry at the end of the queue, sets it's path set to a new temp file, and locks it's mutex
   * 
   * @return pointer to the entry
   */
  MusicStorageEntry *addTempAndLockEntry();

  /** 
   * @brief Adds an entry at the end of the queue and locks it's mutex
   * 
   * @return pointer to the entry
   */
  MusicStorageEntry *addLocalAndLockEntry();


  /**
   * @brief Get the position of an entry in the queue
   * 
   * @param entry pointer to the entry
   * @return the position in the queue (0 being first)
   */
  int getPositionInQueue(const MusicStorageEntry *);

  /**
   * @brief Gets the first song in the list
   * 
   * @return Music* Pointer to the first song in the list
   */
  [[nodiscard]] MusicStorageEntry *getFront();

  /**
   * @brief Get the list of songs
   */
  [[nodiscard]] const std::list<MusicStorageEntry> &getSongs() const;

  /**
   * @brief Removes the first song in the list
   * 
   */
  void removeFront();

   /**
   * @brief Removes a music object by its address
   * 
   * @param musicAddress The address of the music/song 
   */
  void removeByAddress(const MusicStorageEntry *);
  
  /**
   * @brief Removes a music object by its position in queue
   * 
   * @param position The position
   */
  void removeByPosition(uint8_t);

};
