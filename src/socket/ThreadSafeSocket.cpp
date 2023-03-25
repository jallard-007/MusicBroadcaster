/**
 * @author Justin Nicolas Allard
 * Implementation file for socket class
*/

#include <string>
#include <vector>
#include <cstring>
#include <mutex>
#include <iostream>
#include <fstream>
#include <netdb.h>
#include <unistd.h>
#include <sys/socket.h>

#include "ThreadSafeSocket.hpp"
#include "BaseSocket.hpp"

ThreadSafeSocket::ThreadSafeSocket(const int socketFD):
  socketFD{socketFD}, readLock{}, writeLock{} {}

ThreadSafeSocket::ThreadSafeSocket(BaseSocket &&moved):
  socketFD{moved.getSocketFD()}, readLock{}, writeLock{} {
  moved.setSocketFD(0);
}

ThreadSafeSocket::ThreadSafeSocket(ThreadSafeSocket &&moved):
  socketFD{moved.socketFD}, readLock{}, writeLock{} {
  moved.socketFD = 0;
}

ThreadSafeSocket::~ThreadSafeSocket() {
  std::unique_lock<std::mutex> w_lock{writeLock};
  std::unique_lock<std::mutex> r_lock{readLock};
  if (socketFD != 0) {
    close(socketFD);
  }
}

bool ThreadSafeSocket::write(const std::byte *data, const size_t dataSize) const {
  std::unique_lock<std::mutex> lock(writeLock);
  ssize_t numSent = 0;
  if ((numSent = send(socketFD, data, dataSize, 0)) == -1) {
    fprintf(stderr, "send: %s (%d)\n", strerror(errno), errno);
    return false;
  }
  return true;
}

bool ThreadSafeSocket::writeHeaderAndData(const std::byte header[6], const std::byte *data, size_t dataSize) const {
   std::unique_lock<std::mutex> lock(writeLock);
   if (send(socketFD, header, 6, 0) == -1) {
    fprintf(stderr, "send: %s (%d)\n", strerror(errno), errno);
    return false;
  }
  if (send(socketFD, data, dataSize, 0) == -1) {
    fprintf(stderr, "send: %s (%d)\n", strerror(errno), errno);
    return false;
  }
  return true;
}


size_t ThreadSafeSocket::read(std::byte *buffer, const size_t bufferSize) {
  std::unique_lock<std::mutex> lock{readLock};
  const ssize_t numReadBytes = recv(socketFD, buffer, bufferSize, 0);
  if (numReadBytes == -1) {
    fprintf(stderr, "recv: %s (%d)\n", strerror(errno), errno);
    exit(1);
  }
  return static_cast<size_t>(numReadBytes);
}

size_t ThreadSafeSocket::readAll(std::byte *buffer, const size_t bufferSize) {
  std::unique_lock<std::mutex> lock{readLock};
  size_t totalBytesRead = 0;
  do {
    const ssize_t bytesRead = recv(socketFD, buffer + totalBytesRead, bufferSize - totalBytesRead, 0);
    if (bytesRead == -1) {
      fprintf(stderr, "recv: %s (%d)\n", strerror(errno), errno);
      exit(1);
    } else if (bytesRead == 0) {
      return 0;
    }
    totalBytesRead += static_cast<size_t>(bytesRead);
  } while (totalBytesRead < bufferSize);
  if (totalBytesRead != bufferSize) {
    // should not be possible to reach here, but just incase...
    std::cerr << "Error reading from socket. Amount read does not match amount requested to read\n";
    exit(1);
  }
  return bufferSize;
}

int ThreadSafeSocket::getSocketFD() const {
  return socketFD;
}
