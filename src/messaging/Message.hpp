#ifndef MESSAGE_H
#define MESSAGE_H

#include <vector>
#include <cstddef>

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
   * @brief This is the command that the room will use to determine what to do.
   * 
   */
    std::byte command;

    /**
     * @brief This is the options that tells the room what to do with the command. 
     * 
     */
    std::byte options;

    unsigned int bodySize;

  /**
   * @brief 
   * 
   */
    std::vector<std::byte> body;

    /**
     * @brief This will calculate the body size from the body
     * 
     */
    [[nodiscard]] inline unsigned int calculateBodySize() const;

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
  explicit Message(const std::vector<std::byte>& message);

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
  
  /**
   * @brief Get the Command object
   * 
   * @return std::byte
   */
  [[nodiscard]] std::byte getCommand() const;

  /**
   * @brief Get the Options object
   * 
   * @return std::byte
   */
  [[nodiscard]] std::byte getOptions() const;

  /**
   * @brief Get the Body Size object
   * 
   * @return unsigned int
   */
  [[nodiscard]] unsigned int getBodySize() const;

  /**
   * @brief Get the Body object
   * 
   * @return const std::vector<std::byte>& 
   */
  [[nodiscard]] const std::vector<std::byte>& getBody() const;

  /**
   * @brief Set the Command object
   * 
   * @param byte
   */
  void setCommand(std::byte byte);

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
  void setBodySize(unsigned int size);

  /**
   * @brief This will format all the information in the message into a vector of bytes.
   * 
   * @return const std::vector<std::byte> 
   */
  [[nodiscard]] std::vector<std::byte> format() const;

};

#endif
