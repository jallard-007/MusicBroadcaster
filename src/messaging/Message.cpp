/**
 * @file Message.cpp
 * @author Tyler Johnson (tjohn73@uwo.ca)
 * @brief This is the implementation file for the Message class
 * @version 0.1
 * @date 2023-03-07
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#include "Message.hpp"

/**
 * Message Structure
 * 
 * 1 byte command
 * 1 byte options
 * 4 bytes size (N)
 * N bytes body
*/

#include <iostream>

/* Simple constructor */
Message::Message(): contents{SIZE_OF_HEADER, (std::byte)0} {}

Message::Message(const std::byte* header): contents{SIZE_OF_HEADER} {
  for (size_t i = 0; i < SIZE_OF_HEADER; i++) {
    contents[i] = header[i];
  }
}

/* Constructor that takes the binary version of a message */
Message::Message(std::vector<std::byte> &&message): contents{std::move(message)} {}

size_t Message::size() {
  return contents.size();
}

const std::byte *Message::data() {
  return contents.data();
}

/* Gets the command */
Commands::Command Message::getCommand() const {
  return static_cast<Commands::Command>(contents[INDEX_COMMAND]);
}

/* Returns the options */
std::byte Message::getOptions() const {
  return contents[INDEX_OPTION];
}

/* Gets the size of the body */
uint32_t Message::getBodySize() const {
  return *reinterpret_cast<const uint32_t *>(contents.data() + INDEX_START_SIZE);
}

/* Sets the byte */
void Message::setCommand(const std::byte byte) {
  contents[INDEX_COMMAND] = byte;
}

/* Sets the byte */
void Message::setCommand(const Commands::Command command) {
  contents[INDEX_COMMAND] = static_cast<std::byte>(command);
}

/* Sets the byte */
void Message::setOptions(const std::byte byte) {
  contents[INDEX_OPTION] = byte;
}

/* Sets the vector */
void Message::setBody(const std::vector<std::byte>& vector) {
  contents.resize(SIZE_OF_HEADER);
  contents.insert(contents.end(), vector.begin(), vector.end());
  contents.resize(SIZE_OF_HEADER + vector.size());
}

/* Sets the body */
void Message::setBodySize(const uint32_t size) {
  auto p_size = reinterpret_cast<const std::byte *>(&size);
  for (size_t i = 0; i + INDEX_START_SIZE < INDEX_END_SIZE; i++) {
    contents[i + INDEX_START_SIZE] = p_size[i];
  }
}

const std::vector<std::byte> &Message::getMessage() const {
  return contents;
}

const std::byte *Message::getBodyBegin() const {
  return contents.data() + SIZE_OF_HEADER;
}

const std::byte *Message::getBodyEnd() const {
  return contents.end().base();
}

/**
 * @brief we will check if the message is good
 * Don't use this to check if a message is bad. This is not what this is for
 * 
 * @param msg The message to check
 * @return true The message is good
 * @return false The message is not good. Does not mean bad, just means not good
 */
bool Message::good(const Message& msg) {
    return msg.getCommand() == Commands::Command::GOOD_MSG;
}

/**
 * @brief Check if the format was bad
 * Don't use this to check if the message was good. This is not what this means
 * 
 * @param msg The message to check
 * @return true True if the message was of bad format
 * @return false If the format was okay, then it may be something else. Does not mean the message was good
 */
bool Message::badFormat(const Message& msg) {
    return msg.getCommand() == Commands::Command::BAD_FORMAT;
}

/**
 * @brief Check if the values in the message are bad
 * Don't use this to check if the message is good.
 * 
 * @param msg The message to check 
 * @return true If the message contains bad values
 * @return false If the message does not contain bad values
 */
bool Message::badValues(const Message& msg) {
    return msg.getCommand() == Commands::Command::BAD_VALUES;
}