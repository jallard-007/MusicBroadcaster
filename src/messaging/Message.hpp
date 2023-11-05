/**
 * @file Message.hpp
 * @author Justin Nicolas Allard
 * @brief This file will handle the communication between the room and the client. It is the only protocol this project uses
 */
#pragma once

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
 * |- 1 Byte -|- 1 Byte -|- 4 Bytes  -| 
 * |----------------------------------| 
 * | Command  |  Options | File Size  | 
 * |----------------------------------| 
 * |              BODY                | <- Body is the size of defined by the file size 
 * |----------------------------------| 
 * 
 * The command is a single byte that tells the room what to do. The options is a single byte that tells the room what the command should do about the body.
 * The body size is a 4 byte unsigned integer that tells the room how big the body is. The body contains the data that the command will act on or use.
 */
class Message {

private:

    /**
     * @brief This holds the contents of the message body
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
   * @return Commands::Command
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
  [[nodiscard]] const std::vector<std::byte> &getMessage() const;

  /**
   * @return pointer to start of the body
  */
  [[nodiscard]] const std::byte *getBodyBegin() const;
  /**
   * content.end().base()
   * @return pointer to the element one after the last in this message
   * 
  */
  [[nodiscard]] const std::byte *getBodyEnd() const;

  /**
   * @brief we will check if the message is good
   * Don't use this to check if a message is bad. This is not what this is for
   * Only use this for a response message. Some messages are "good", but this is to check for
   * good response messages
   * 
   * @param msg The message to check
   * @return true The message is good
   * @return false The message is not good. Does not mean bad, just means not good
   */
  static bool good(const Message& msg);

  /**
   * @brief Check if the format was bad
   * Don't use this to check if the message was good. This is not what this means.
   * Use this only for response messages
   * 
   * @param msg The message to check
   * @return true True if the message was of bad format
   * @return false If the format was okay, then it may be something else. Does not mean the message was good
   */
  static bool badFormat(const Message& msg);

  /**
   * @brief Check if the values in the message are bad
   * Don't use this to check if the message is good.
   * Use this only for response messages
   * 
   * @param msg The message to check 
   * @return true If the message contains bad values
   * @return false If the message does not contain bad values
   */
  static bool badValues(const Message& msg);

};

