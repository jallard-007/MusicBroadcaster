/**
 * \author Justin Nicolas Allard
 * Header file for client class
*/

#ifndef CLIENT_CLASS_H
#define CLIENT_CLASS_H

#include <string>
#include "../socket/BaseSocket.hpp"

class Client {
private:
  int id;
  std::string clientName;
  BaseSocket clientSocket;
public:
  Client() = default;
  ~Client() = default;

  /**
   * Constructor
   * \param name name of client
  */
  Client(const std::string &name);

  /**
   * Constructor
   * \param name name of client
   * \param socket socket associated with this client
  */
  Client(const std::string &name, const BaseSocket &socket);

  /**
   * Asks for the path to the file.
   * Then sends the size of the file as well as the file itself through clientSocket
  */
  void sendMusicFile();

  [[nodiscard]] const BaseSocket &getSocket() const;
  [[nodiscard]] const std::string &getName() const;
  void setName(std::string);
  int getId() const;
};

#endif
