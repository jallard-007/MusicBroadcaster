/**
 * @author Justin Nicolas Allard
 * Implementation file for player class
*/

#include <ao/ao.h>
#include <mpg123.h>
#include <thread>
#include <cstddef>
#include <iostream>
#include "Player.hpp"
#include "Music.hpp"

Player::Player(): driver{}, shouldPlay{false}, outBufferSize{0},
  outBuffer{nullptr}, mh{}, dev{nullptr}, player{}
{
  ao_initialize();
  driver = ao_default_driver_id();
  mh = mpg123_new(NULL, NULL);
  mpg123_param(mh, MPG123_ADD_FLAGS, MPG123_IGNORE_STREAMLENGTH, 0); // removes warning message on 'Frankenstein'
  mpg123_open_feed(mh);
  outBufferSize = mpg123_outblock(mh);
  outBuffer = malloc(outBufferSize);
}

Player::~Player() {
  ao_close(dev);
  mpg123_delete(mh);
  ao_shutdown();
  free(outBuffer);
}

void Player::feed(unsigned char *food, std::size_t foodSize) {
  mpg123_feed(mh, food, foodSize);
}

void Player::feed(Music *music) {
  feed((unsigned char *)music->getBytes().data(), music->getBytes().size());
}

void Player::_newFormat() {
  int channels, encoding;
  long rate;
  mpg123_getformat(mh, &rate, &channels, &encoding);
  ao_sample_format format = {(int)mpg123_encsize(encoding) * BITS, (int)rate, channels, AO_FMT_NATIVE, 0};
  if (dev != nullptr) {
    ao_close(dev);
  }
  dev = ao_open_live(driver, &format, NULL);
}

void Player::play() {
  if (shouldPlay) {
    std::cerr << "Already playing\n";
    return;
  }
  wait();
  std::cout << "Playing...\n";
  shouldPlay = true;
  player = std::thread(&Player::_play, this);
}

void Player::pause() {
  shouldPlay = false;
  wait();
}

void Player::wait() {
  if (player.joinable()) {
    player.join();
  }
}

void Player::_play() {
  std::size_t done;
  while (shouldPlay) {
    // decode audio
    const int err = mpg123_read(mh, outBuffer, outBufferSize, &done);
    if (err == MPG123_NEW_FORMAT) {
      // new format as been detected, handle it
      _newFormat();
    } else if (err != MPG123_OK) {
      // this most likely means that there is no more audio to read but could be something else
      shouldPlay = false;
    }
    // play the audio
    ao_play(dev, (char *)outBuffer, (uint_32)done);
  }
}
