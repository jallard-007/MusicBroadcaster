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
Message::Message() {
    this->command = (std::byte)0;
    this->options = (std::byte)0;
    this->bodySize = 0;
}

Message::Message(const std::byte* header) {
    this->command = header[0];
    this->options = header[1];
    this->bodySize = *reinterpret_cast<const uint32_t *>(header + 2);
}

/* Constructor that takes the binary version of a message */
Message::Message(const std::vector<std::byte>& message) {
    
    /* Get the command from the first byte */
    this->command = message[0];

    /* Get the options from the second */
    this->options = message[1];

    this->bodySize = static_cast<uint32_t>(message.size());

    /* Get the body from the rest of the message */
    this->body.insert(this->body.begin(), message.begin() + 6, message.end());

}

/* Gets the command */
std::byte Message::getCommand() const {
    return this->command;
}

/* Returns the options */
std::byte Message::getOptions() const {
    return this->options;
}

/* Gets the size of the body */
uint32_t Message::getBodySize() const {
    return bodySize;
}

/* Get the body of the message */
const std::vector<std::byte>& Message::getBody() const {
    return this->body;
}

/* Sets the byte */
void Message::setCommand(const std::byte byte) {
    this->command = byte;
}

/* Sets the byte */
void Message::setOptions(const std::byte byte) {
    this->options = byte;
}

/* Sets the vector */
void Message::setBody(const std::vector<std::byte>& vector) {
    this->body = vector;
}

/* Sets the body */
void Message::setBodySize(const uint32_t size) {
    this->bodySize = size;
}

inline uint32_t Message::calculateBodySize() const {
    return (uint32_t)this->body.size();
}

/* Formats the message into a vector of bytes */
std::vector<std::byte> Message::format() const {

    /* Create a vector of bytes to hold the message */
    std::vector<std::byte> message;

    /* Add the command */
    message.push_back(this->command);

    /* Add the options */
    message.push_back(this->options);

    /* Add the body size */
    for (size_t i = 0; i < 4; i++) 
        message.push_back(((std::byte *)&bodySize)[i]);

    /* Add the body */
    message.insert(message.end(), this->body.begin(), this->body.end());

    /* Return the message */
    // previously returned a reference to local variable, no good, now potentially copying entire vector
    // maybe use std::vector<std::byte> as member variable and then return reference to that.
    // specific getters would then return a specific byte from that vector
    // example: std::byte getCommand() { return message[0]; }
    return message;

}
