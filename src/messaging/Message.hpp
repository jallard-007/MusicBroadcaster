#ifndef MESSAGE_H
#define MESSAGE_H

#include <vector>
#include <cstddef>
#include <cstdint>

#include "Commands.hpp"

#define INDEX_COMMAND 0
#define INDEX_OPTION 1
#define INDEX_START_SIZE 2
#define INDEX_END_SIZE 6
#define SIZE_OF_HEADER 6

/**
 * @brief This class will handle the communication between the room and the client.
 * Looking at the protocol, the message is made up of 4 parts:
 * 1. Command
 * 2. Options
 * 3. Body Size
 * 4. Body
 * 
 * The command is a single byte that tells the room what to do. The options is a single byte that tells the room what the command should do about the body.
 * The body size is a 4 byte unsigned integer that tells the room how big the body is. The body contains the data that the command will act on or use.
 */
class Message {

private:
    /**
     *
     * 
     */
    std::vector<std::byte> contents;

public:

  /**
   * @brief Construct a new Message object
   * 
   */
  Message();

  /**
   * @brief Construct a new Message object
   * 
   */
  explicit Message(const std::byte *header);

  /**
   * @brief Construct a new Message object with the given message in bytes
   * 
   * @param message This a message in binary data
   */
  explicit Message(std::vector<std::byte> &&message);

  /**
   * @brief Destroy the Message object
   * 
   */
  ~Message() = default; 

  /**
   * @brief Copy over the message object
   * 
   * @param other 
   */
  Message(const Message& other) = default;

  /**
   * @brief Move the message object
   * 
   * @param other 
   */
  Message(Message&& other) = default;

  size_t size();

  const std::byte *data();
  
  /**
   * @brief Get the Command object
   * 
   * @return std::byte
   */
  [[nodiscard]] Commands::Command getCommand() const;

  /**
   * @brief Get the Options object
   * 
   * @return std::byte
   */
  [[nodiscard]] std::byte getOptions() const;

  /**
   * @brief Get the Body Size object
   * 
   * @return uint32_t
   */
  [[nodiscard]] uint32_t getBodySize() const;

  /**
   * @brief Set the Command object
   * 
   * @param byte
   */
  void setCommand(std::byte byte);

  /**
   * @brief Set the Command object
   * 
   * @param command
   */
  void setCommand(Commands::Command command);

  /**
   * @brief Set the Options object
   * 
   * @param byte
   */
  void setOptions(std::byte byte);

  /**
   * @brief Set the Body object
   * 
   * @param vector
   */
  void setBody(const std::vector<std::byte>& vector);

    /**
   * @brief Set the BodySize object
   * 
   * @param size
   */
  void setBodySize(uint32_t size);

  /**
   * @return const std::vector<std::byte> 
   */
  [[nodiscard]] const std::vector<std::byte> &getMessage();

};

#endif
