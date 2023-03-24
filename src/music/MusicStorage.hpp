/**
 * @author Justin Nicolas Allard
 * Header file for music storage class
*/

#ifndef MUSIC_STORAGE_H
#define MUSIC_STORAGE_H

#include <list>
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
  std::list<Music> songs;

public:
  MusicStorage();
  ~MusicStorage() = default;

  void removeByPath(const std::string &);
  void removeByName(const std::string &);

  /**
   * Removes a music object by its address in memory
  */
  void removeByAddress(Music *);

  /**
   * Add an unnamed Music object to the back of the list
   * @returns pointer to the new Music object, nullptr if no room
  */
  Music *add();

  /**
   * Adds a Music object to the list
   * @param musicName name of music
   * @returns pointer to the new Music object, nullptr if no room
   */
  Music *addByName(const std::string &);

  /**
   * Gets a music object by its path property
   * @returns pointer to object, nullptr if the object is not in the list
   */
  [[nodiscard]] Music *getByPath(const std::string &);

  /**
   * Gets a music object by its name property
   * @returns pointer to object, nullptr if the object is not in the list
  */
  [[nodiscard]] Music *getByName(const std::string &);

  [[nodiscard]] Music *getFront();
  void removeFront();
};

#endif
