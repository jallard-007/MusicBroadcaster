/**
 * @author Justin Nicolas Allard
 * Implementation file for music class
*/

#include <iostream>
#include <fstream>
#include <utility>
#include <vector>
#include <unistd.h>
#include "Music.hpp"

Music::Music(std::string songName):
  name{std::move(songName)}, path{}, bytes{} {}

Music::Music(
  std::string songName,
  std::string filePath
): name{std::move(songName)}, path{std::move(filePath)}, bytes{} {}

Music::Music(
  std::string songName,
  const std::vector<std::byte> &buffer
): name{std::move(songName)}, path{}, bytes{buffer} {}

bool Music::readFileAtPath() {
  FILE *fp = fopen(path.c_str(), "r");
  if (fp == nullptr) {
    std::cerr << "Error: Unable to open file\n";
    return false;
  }
  fseek(fp, 0L, SEEK_END); // go to the end of the file
  const auto fileSize = (size_t)ftell(fp); // tell us the current position (this tells us the size of the file)
  if (fileSize > MAX_FILE_SIZE_BYTES) {
    std::cerr << "Error: File too big. Max file size is " << (MAX_FILE_SIZE_BYTES / 1000000.0) << " megabytes\n";
    return false;
  }
  if (fileSize == 0) {
    std::cerr << "Error: File empty\n";
    return false;
  }

  bytes.resize(fileSize); // allocate memory to fit the file
  
  fseek(fp, 0L, SEEK_SET); // reset position to beginning of file
  fread((void *)bytes.data(), bytes.size(), 1, fp); // read the file
  fclose(fp);
  return true;
}

void Music::writeToPath() {
  FILE *fp = fopen(path.c_str(), "w");
  if (fp == nullptr) {
    return;
  }
  fwrite(bytes.data(), bytes.size(), 1, fp);
  fclose(fp);
}

std::shared_ptr<Music> Music::getMemShared() const {
  auto music = std::make_shared<Music>();
  music.get()->setPath(path);
  music.get()->readFileAtPath();
  return music;
}

const std::string &Music::getName() const {
  return name;
}

const std::string &Music::getPath() const {
  return path;
}

std::vector<std::byte> &Music::getVector() {
  return bytes;
}

void Music::setPath(const std::string &newPath) {
  path = newPath;
}

void Music::setVector(const std::vector<std::byte> &musicBytes) {
  bytes = musicBytes;
}
