/**
 * @author Justin Nicolas Allard
 * Header file for ThreadSafe socket class
*/

#ifndef THREAD_SAFE_SOCKET_H
#define THREAD_SAFE_SOCKET_H

#include <cstddef>
#include <string>
#include <vector>
#include <mutex>

#include "BaseSocket.hpp"

/**
 * Used to communicate over a network. Same as BaseSocket class, but adds a mutex lock to reading and writing
*/
class ThreadSafeSocket {
private:

  /**
   * file descriptor in which to read and write to
  */
  int socketFD;

  /**
   * lock for reading
  */
  mutable std::mutex readLock;

  /**
   * lock for writing
  */
  mutable std::mutex writeLock;

public:
  ThreadSafeSocket() = default;

  /**
   * Constructor which takes a file descriptor for the socket
   * @param socketFD file descriptor
  */
  ThreadSafeSocket(int socketFD);

  /**
   * Move BaseSocket object into ThreadSafe
  */
  ThreadSafeSocket(BaseSocket &&);

  /**
   * Copy constructor. deleted since the destructor closes the socket, use std::move to assign instead
   */
  ThreadSafeSocket(const ThreadSafeSocket &) = delete;

  /**
   * Move constructor. sets original object's file descriptor to 0 so that it does not close the original socket
   */
  ThreadSafeSocket(ThreadSafeSocket &&);

  /**
   * Destructor. Closes the socket
  */
  ~ThreadSafeSocket();

  /**
   * Getter for socketFD
  */
  int getSocketFD() const;

  /**
   * Write raw data to socketFD
   * @param data pointer to data
   * @param dataSize size of data
   * @returns true if successfully wrote data, false on error
  */
  bool write(const std::byte *data, size_t dataSize) const;

    /**
   * Write raw data to socketFD
   * @param data pointer to data
   * @param dataSize size of data
   * @returns true if successfully wrote data, false on error
  */
  bool writeHeaderAndData(const std::byte header[6], const std::byte *data, size_t dataSize) const;

  /**
   * Read raw data from socketFD, might not read all bytes
   * @param buffer pointer to buffer to write to
   * @param bufferSize size of buffer
   * @returns number of bytes read or 0 if socket closed by peer
  */
  size_t read(std::byte *buffer, size_t bufferSize);

  /**
   * Reads raw data from socketFD. Will ensure all bytes are read.
   * Should use this rather than ThreadSafeSocket::read in most cases.
   * @param buffer pointer to buffer to write to
   * @param bufferSize size of buffer
   * @returns bufferSize or 0 if the socket was closed by peer
  */
  size_t readAll(std::byte *buffer, size_t bufferSize);
};

#endif
