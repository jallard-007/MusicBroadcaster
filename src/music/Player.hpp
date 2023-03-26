/**
 * @author Justin Nicolas Allard
 * Header file for player class
*/

#ifndef PLAYER_H
#define PLAYER_H

#include <ao/ao.h>
#include <mpg123.h>
#include <thread>
#include <atomic>
#include <cstddef>
#include "Music.hpp"

#define BITS 8
#define MP3_FRAMES_PER_SEC 38.46

class Player {
private:
  /**
   * used by libao
  */
  int driver;

  /**
   * atomic boolean to communicate with player thread
  */
  std::atomic<bool> shouldPlay; 

  /**
   * size of decode buffer
  */
  std::size_t outBufferSize;

  /**
   * buffer in which decoded samples go
  */
  void *outBuffer;

  /**
   * used by mpg123
  */
  mpg123_handle *mh;

  /**
   * used by libao
  */
  ao_device *dev;

  /**
   * thread in which audio plays
  */
  std::thread player;

public:
  Player();
  ~Player();

  /**
   * feed audio data to mpg123
   * @param fp pointer to FILE containing audio data
  */
  void feed(FILE *fp);

  /**
   * plays audio which as been fed to mpg123
  */
  void play();

  /**
   * pauses audio
  */
  void pause();

  /**
   * waits for the player thread to finish execution (no more audio to play, or some error occurs)
  */
  void wait();

  void seek(float time);

  void clear();

private:
  void _play();
  void _newFormat();
};

#endif
