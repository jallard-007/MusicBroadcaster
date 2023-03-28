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
class MusicStorage {
private:

  /**
   * we dont need the music objects to be next to each other, so use a list
  */
  std::list<std::string> songs;

public:
  MusicStorage();
  ~MusicStorage() = default;

  /**
   * Removes a music object by its address in memory
  */
  void removeByAddress(const std::string *);

  /**
   * Add an unnamed Music object to the back of the list
   * @returns pointer to the new Music object, nullptr if no room
  */
  const std::string *add();

  [[nodiscard]] const std::string *getFront();
  [[nodiscard]] std::shared_ptr<Music> getFrontMem();

  void removeFront();
};

#endif
