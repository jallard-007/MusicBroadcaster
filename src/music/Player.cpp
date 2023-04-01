/**
 * @author Justin Nicolas Allard
 * Implementation file for player class
*/

#include <out123.h>
#include <mpg123.h>
#include <thread>
#include <iostream>
#include "Player.hpp"

Player::Player(): shouldPlay{} {
  mh = mpg123_new(nullptr, nullptr);
  if (mh == nullptr) {
    std::cerr << "Error: mpg123 library failed\n";
    exit(1);
  }
  ao = out123_new();
  if (ao == nullptr) {
    std::cerr << "Error: out123 library failed\n";
    exit(1);
  }
  mpg123_param(mh, MPG123_ADD_FLAGS, MPG123_IGNORE_STREAMLENGTH, 0); // removes warning message on 'Frankenstein'
  outBufferSize = mpg123_outblock(mh);
  outBuffer = malloc(outBufferSize);
  if(out123_open(ao, nullptr, nullptr) != OUT123_OK) {
    std::cout << "err: " << out123_strerror(ao) << '\n';
    exit(1);
  }
}

Player::~Player() {
  shouldPlay = false;
  out123_del(ao);
  mpg123_delete(mh);
  free(outBuffer);
}

void Player::feed(const char *fp) {
  pause();
  mpg123_open(mh, fp);
}

void Player::_newFormat() {
  int channels, encoding;
  long rate;
  if (mpg123_getformat(mh, &rate, &channels, &encoding) != MPG123_OK) {
    std::cout << "err: " << mpg123_strerror(mh) << '\n';
    exit(1);
  }
  if (out123_start(ao, rate, channels, encoding) != OUT123_OK) {
    std::cout << "err: " << out123_strerror(ao) << '\n';
    exit(1);
  }
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
  size_t played;
  int err;
  while (shouldPlay) {
    // decode audio
    err = mpg123_read(mh, outBuffer, outBufferSize, &done);
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
    if ((played = out123_play(ao, outBuffer, done)) != done) {
      // try to finish playing
      //out123_play(ao, outBuffer + played, done - played);
    }
  }
}
