/**
 * @author Justin Nicolas Allard
 * Implementation file for player class
*/

#include <ao/ao.h>
#include <mpg123.h>
#include <thread>
#include <iostream>
#include "Player.hpp"
#include "Music.hpp"

Player::Player(): shouldPlay{false}, dev{nullptr}, player{} {
  ao_initialize();
  driver = ao_default_driver_id();
  mh = mpg123_new(nullptr, nullptr);
  mpg123_param(mh, MPG123_ADD_FLAGS, MPG123_IGNORE_STREAMLENGTH, 0); // removes warning message on 'Frankenstein'
  outBufferSize = mpg123_outblock(mh);
  outBuffer = malloc(outBufferSize);
}

Player::~Player() {
  shouldPlay = false;
  wait();
  ao_close(dev);
  mpg123_delete(mh);
  ao_shutdown();
  free(outBuffer);
}

void Player::feed(const char *fp) {
  pause();
  mpg123_open(mh, fp);
}

void Player::_newFormat() {
  int channels, encoding;
  long rate;
  mpg123_getformat(mh, &rate, &channels, &encoding);
  ao_sample_format format = {(int)mpg123_encsize(encoding) * BITS, (int)rate, channels, AO_FMT_NATIVE, nullptr};
  if (dev != nullptr) {
    ao_close(dev);
  }
  dev = ao_open_live(driver, &format, nullptr);
}

void Player::play() {
  if (shouldPlay) {
    return;
  }
  wait();
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

bool Player::isPlaying() {
  return shouldPlay;
}

void Player::seek(float time) {
  const auto offset = static_cast<off_t>(time * MP3_FRAMES_PER_SEC);
  if (mpg123_seek_frame(mh, offset, SEEK_SET) < 0) {
    mpg123_strerror(mh);
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
    } else if (err == MPG123_DONE) {
      shouldPlay = false;
    } else if (err != MPG123_OK) {
      std::cout << "err: " << mpg123_strerror(mh) << '\n';
      // some error
      shouldPlay = false;
    }
    // play the audio
    ao_play(dev, (char *)outBuffer, (uint_32)done);
  }
}
