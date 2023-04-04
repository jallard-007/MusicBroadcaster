/**
 * @file Player.hpp
 * @author Justin Nicolas Allard
 * @brief This plays the song
 * @version 1.3
 * @date 2023-04-02
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#pragma once

#include <out123.h>
#include <mpg123.h>
#include <thread>
#include <atomic>
#include <cstddef>
#include <iostream>

#include "Music.hpp"
#include "../debug.hpp"

#define BITS 8

class Player {
private:

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
   * used by out123
  */
  out123_handle *ao;

  /**
   * used by mpg123
  */
  mpg123_handle *mh;

  /**
   * thread in which audio plays
  */
  std::thread player;

public:
  
  /**
   * @brief Construct a new Player object
   * 
   */
  Player(); 

  /**
   * @brief Destroy the Player object
   * 
   */
  ~Player();

  /**
   * @brief feed audio data to mpg123
   * @param fp path to file, ex: /tmp/musicBroadcaster_XXXXX
  */
  void feed(const char *fp);

  /**
   * @brief plays audio which as been fed to mpg123
  */
  void play();

  /**
   * @brief pauses audio
  */
  void pause();

  /**
   * @brief waits for the player thread to finish execution (no more audio to play, or some error occurs)
  */
  void wait();

  /**
   * @brief mute audio
  */
  void mute();

  /**
   * @brief unmute audio
  */
  void unmute();

  /**
   * @brief seek to a time in the track
   * @param time time in seconds to seek to
  */
  void seek(double time);

  /**
   * @brief check if audio is playing
  */
  bool isPlaying();

private:
  /**
   * @brief actually plays audio
  */
  void _play();

  /**
   * @brief handles a new mp3 format
  */
  void _newFormat();
};
