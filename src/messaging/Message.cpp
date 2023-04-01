#include <iostream>
#include "Message.hpp"

/**
 * Message Structure
 * 
 * 1 byte command
 * 1 byte options
 * 4 bytes size (N)
 * N bytes body
*/

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
    contents.resize(SIZE_OF_HEADER + vector.size());
    contents.insert(contents.begin() + 6, vector.begin(), vector.end());
}

/* Sets the body */
void Message::setBodySize(const uint32_t size) {
    auto p_size = reinterpret_cast<const std::byte *>(&size);
    for (size_t i = 0; i < INDEX_END_SIZE; i++) {
        contents[i + INDEX_START_SIZE] = p_size[i];
    }
}

const std::vector<std::byte> &Message::getMessage() {
    return contents;
}
