/**
 * @file Music.hpp
 * @author Justin Nicolas Allard
 * @brief Header file for music class
 * @version 1.6
 * @date 2023-04-02
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#pragma once

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <utility>
#include <memory>

#define MAX_FILE_SIZE_BYTES 50000000 // 50 megabytes. can be changed to whatever

/**
 * @brief Music class. Handles the music
 * 
 */
class Music {

private:

  /**
   * @brief The name of the music
   * 
   */
  std::string name;

  /**
   * @brief The path of the music
   * 
   */
  std::string path;

  /**
   * @brief The bytes of the music
   * 
   */
  std::vector<std::byte> bytes;

  /**
   * @brief validates a file and prints errors, if any
   */
  static size_t validateFile(FILE *fp);

  /**
   * @brief validates a file, does not print errors
   */
  static size_t validateFileSilent(FILE *fp);

public:
  /**
   * @brief Construct a new Music object
   * 
   */
  Music() = default;
  ~Music() = default;

  /**
   * Attempts to read the file at Music::path and store the contents in Music::bytes
   * @returns true on success, false otherwise
  */
  bool readFileAtPath(size_t (*)(FILE *) = &validateFile);

  /**
   * Attemps to read the file at Music::path and store the contents in Music::bytes
   * @returns true on success, false otherwise
  */

  void writeToPath();
  /**
   * Setters and getters
  */

  bool validateFileAtPath();
 

  /**
   * @brief Get the name of the song
   * 
   * @return const std::string& 
   */
  [[nodiscard]] const std::string &getName() const;


  /** 
   * @brief Get the path to the file
   * 
   * @return const std::string& 
   */
  [[nodiscard]] const std::string &getPath() const;

  /**
   * @brief Get the vector of this object
   * 
   * @return std::vector<std::byte>& 
   */
  [[nodiscard]] std::vector<std::byte> &getVector();

  /**
   * @brief Loads the file, at 'path', into memory and makes a shared pointer to it
   * 
   * @returns std::shared_ptr<Music>
   */
  [[nodiscard]] std::shared_ptr<Music> getMemShared() const;

  /**
   * @brief Set the path of the song
   * 
   * @param newPath The new path for the song
   * 
   */
  void setPath(const std::string &);

  /**
   * @brief Set the bytes of the song
   * 
   * @param musicBytes The new bytes for the song
   * 
   */
  void setVector(const std::vector<std::byte> &);


};
