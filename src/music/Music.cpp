/**
 * @author Justin Nicolas Allard
 * Implementation file for music class
*/

#include <iostream>
#include <fstream>
#include <iterator>
#include <vector>
#include "Music.hpp"

Music::Music(const std::string &songName):
  name{songName}, path{}, bytes{} {}

Music::Music(
  const std::string &songName,
  const std::string &filePath
): name{songName}, path{filePath}, bytes{} {}

Music::Music(
  const std::string &songName,
  const std::vector<std::byte> &buffer
): name{songName}, path{}, bytes{buffer} {}

const std::string &Music::getName() const {
  return name;
}

const std::string &Music::getPath() const {
  return path;
}

std::vector<std::byte> &Music::getBytes() {
  return bytes;
}

void Music::setPath(const std::string &newPath) {
  path = newPath;
}

void Music::setBytes(const std::vector<std::byte> &musicBytes) {
  bytes = musicBytes;
}

void Music::appendBytes(const std::vector<std::byte> &musicBytes) {
  bytes.insert(bytes.end(), musicBytes.begin(), musicBytes.end());
}

bool Music::readFileAtPath() {
  FILE *fp = fopen(path.c_str(), "r");
  if (fp == nullptr) {
    std::cerr << "Error: Unable to open file\n";
    return false;
  }
  fseek(fp, 0L, SEEK_END); // go to the end of the file
  const size_t fileSize = (size_t)ftell(fp); // tell us the current position (this tells us the size of the file)
  if (fileSize > MAX_FILE_SIZE_BYTES) {
    std::cerr << "Error: File too big. Max file size is " << (MAX_FILE_SIZE_BYTES / 1000000.0) << " megabytes\n";
    fclose(fp);
    return false;
  }

  bytes.resize(fileSize); // allocate memory to fit the file
  
  fseek(fp, 0L, SEEK_SET); // reset position to beginning of file
  fread((void *)bytes.data(), (size_t)fileSize, 1, fp); // read the file
  fclose(fp); // close the file

  return true;
}
