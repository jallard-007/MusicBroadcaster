/**
 * @author Justin Nicolas Allard
 * Implementation file for player class
*/

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
  pause();
  wait();
  out123_del(ao);
  mpg123_delete(mh);
  free(outBuffer);
}

void Player::feed(const char *fp) {
  pause();
  mpg123_open(mh, fp);
  _newFormat();
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

void Player::mute() {
  out123_param(ao, OUT123_ADD_FLAGS, OUT123_MUTE, 0, nullptr);
}

void Player::unmute() {
  out123_param(ao, OUT123_REMOVE_FLAGS, OUT123_MUTE, 0, nullptr);
}

bool Player::isPlaying() {
  return shouldPlay;
}

void Player::seek(double time) {
  DEBUG_P(std::cout << "seek to time: " << time << '\n');
  if (mpg123_seek_frame(mh, mpg123_timeframe(mh, time), SEEK_SET) < 0) {
    mpg123_strerror(mh);
  }
}

void Player::_play() {
  std::size_t done;
  int err;
  while (shouldPlay) {
    // decode audio
    err = mpg123_read(mh, outBuffer, outBufferSize, &done);
    if (err == MPG123_NEW_FORMAT) {
      // new format as been detected, handle it
      _newFormat();
      continue;
    } else if (err == MPG123_DONE) {
      shouldPlay = false;
    } else if (err != MPG123_OK) {
      std::cout << "err: " << mpg123_strerror(mh) << '\n';
      // some error
      shouldPlay = false;
    }
    // play the audio
    if (out123_play(ao, outBuffer, done) != done) {
      // try to finish playing
      // out123_play(ao, outBuffer + played, done - played);
    }
  }
}
