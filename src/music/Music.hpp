/**
 * @author Justin Nicolas Allard
 * Header file for music class
*/

#ifndef MUSIC_CLASS_H
#define MUSIC_CLASS_H

#include <string>
#include <vector>

#define MAX_FILE_SIZE_BYTES 50000000 // 50 megabytes. can be changed to whatever

class Music {
private:
  std::string name;
  std::string path;
  std::vector<std::byte> bytes;
public:
  Music() = default;
  ~Music() = default;
  
  explicit Music(std::string );

  /**
   * Used to create an instance with a local file
   */
  Music(std::string name, std::string path);

  /**
   * Used to create an instance with a downloaded stream
   */
  Music(std::string , const std::vector<std::byte> &);

  /**
   * Attemps to read the file at Music::path and store the contents in Music::bytes
   * @returns true on success, false otherwise
  */
  bool readFileAtPath();

  /**
   * Setters and getters
  */
 
  [[nodiscard]] const std::string &getName() const;
  [[nodiscard]] const std::string &getPath() const;
  [[nodiscard]] std::vector<std::byte> &getBytes();
  void setPath(const std::string &);
  void setBytes(const std::vector<std::byte>&);
  void appendBytes(const std::vector<std::byte>&);
  
};

#endif
