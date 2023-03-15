/**
 * \author Justin Nicolas Allard
 * Header file for socket class
*/

#ifndef BASE_SOCKET_H
#define BASE_SOCKET_H

#include <cstddef>
#include <string>
#include <vector>

/**
 * Used to communicate over a network
*/
class BaseSocket {
private:

  /**
   * file descriptor in which to read and write to
  */
  int socketFD;
public:
  BaseSocket() = default;

  /**
   * Constructor which takes a file descriptor for the socket
   * \param socketFD file descriptor
  */
  BaseSocket(int socketFD);

  /**
   * Copy constructor
   */
  BaseSocket(const BaseSocket &);

  /**
   * Move constructor
   */
  BaseSocket(BaseSocket &&);

  /**
   * Destructor. Closes the socket
  */
  ~BaseSocket();

  /**
   * Getter for socketFD
  */
  int getSocketFD() const;

  /**
   * Setter for socketFD
   */
  void setSocketFD(int);
  
  /**
   * Attempts to connect via IP and port to another TCP socket
   * \param ip ip address, can be numerical or domain name.
   * Example 0, 127.0.0.1, and localhost all do the same thing
   * \param port port number of process
   * \returns true on successful connection, false on error
  */
  bool connect(const std::string &ip, uint16_t port);

  /**
   * Attempts to bind IP and port to socket
   * \param ip ip address, can be numerical or domain name.
   * Example 0, 127.0.0.1, and localhost all do the same thing
   * \param port port number of process
  */
  void bind(const std::string &, uint16_t);

  /**
   * Call after BaseSocket::bind() to allow incoming requests to connect
  */
  void listen(int backlog = 20);

  /**
   * Accepts connection requests. This is blocking, current thread will wait til a request comes through
   * \returns file descriptor of socket
  */
  int accept();

  /**
   * Write raw data to socketFD
   * \param data pointer to data
   * \param dataSize size of data
   * \returns true if successfully wrote data, false on error
  */
  bool write(const std::byte *data, size_t dataSize) const;

  /**
   * Read raw data from socketFD
   * \param buffer pointer to buffer to write to
   * \param bufferSize size of buffer
   * \returns number of bytes read
  */
  size_t read(std::byte *buffer, size_t bufferSize);
};

#endif
