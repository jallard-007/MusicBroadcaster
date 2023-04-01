/**
 * @author Justin Nicolas Allard
 * Implementation file for music class
*/

#include <iostream>
#include <fstream>
#include <utility>
#include <vector>
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

size_t Music::validateFile(FILE *fp) {
  if (fp == nullptr) {
    std::cerr << "Error: Unable to open file\n";
    return 0;
  }
  fseek(fp, 0L, SEEK_END); // go to the end of the file
  const auto fileSize = (size_t)ftell(fp); // tell us the current position (this tells us the size of the file)
  if (fileSize > MAX_FILE_SIZE_BYTES) {
    std::cerr << "Error: File too big. Max file size is " << (MAX_FILE_SIZE_BYTES / 1000000.0) << " megabytes\n";
    return 0;
  }
  if (fileSize == 0) {
    std::cerr << "Error: File empty\n";
    return 0;
  }
  return fileSize;
}

size_t Music::validateFileSilent(FILE *fp) {
  if (fp == nullptr) {
    return 0;
  }
  fseek(fp, 0L, SEEK_END); // go to the end of the file
  const auto fileSize = (size_t)ftell(fp); // tell us the current position (this tells us the size of the file)
  if (fileSize > MAX_FILE_SIZE_BYTES) {
    return 0;
  }
  if (fileSize == 0) {
    return 0;
  }
  return fileSize;
}

bool Music::validateFileAtPath() {
  FILE *fp = fopen(path.c_str(), "r");
  
  size_t valid = validateFile(fp);
  if (fp != nullptr) {
    fclose(fp);
  }
  return valid > 0;
}

bool Music::readFileAtPath(size_t (*validator)(FILE *)) {
  FILE *fp = fopen(path.c_str(), "r");
  size_t fileSize = validator(fp);
  if (fileSize == 0) {
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
  music->setPath(path);
  if (music->readFileAtPath(&Music::validateFileSilent) == 0) {
    return nullptr;
  }
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
