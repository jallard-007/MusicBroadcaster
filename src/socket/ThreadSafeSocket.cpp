/**
 * @author Justin Nicolas Allard
 * Implementation file for socket class
*/

#include <string>
#include <vector>
#include <cstring>
#include <mutex>
#include <iostream>
#include <netdb.h>
#include <unistd.h>
#include <sys/socket.h>

#include "ThreadSafeSocket.hpp"
#include "BaseSocket.hpp"

ThreadSafeSocket::ThreadSafeSocket():
        socketFD{}, readLock{}, writeLock{} {}

ThreadSafeSocket::ThreadSafeSocket(const int socketFD):
  socketFD{socketFD}, readLock{}, writeLock{} {}

ThreadSafeSocket::ThreadSafeSocket(BaseSocket &&moved):
  socketFD{moved.getSocketFD()}, readLock{}, writeLock{} {
  moved.setSocketFD(0);
}

ThreadSafeSocket::ThreadSafeSocket(ThreadSafeSocket &&moved) noexcept:
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

bool ThreadSafeSocket::connect(const std::string &ip, const uint16_t port) {
  std::unique_lock<std::mutex> w_lock{writeLock};
  std::unique_lock<std::mutex> r_lock{readLock};
  struct addrinfo hints{};
  struct addrinfo *res;  // will point to the results
  memset(&hints, 0, sizeof hints); // make sure the struct is empty
  hints.ai_family = AF_UNSPEC;     // don't care IPv4 or IPv6
  hints.ai_socktype = SOCK_STREAM; // TCP stream sockets
  // get ready to connect
  if (getaddrinfo(ip.c_str(), std::to_string(port).c_str(), &hints, &res) == -1) {
    fprintf(stderr, "getaddrinfo: %s (%d)\n", strerror(errno), errno);
    return false;
  }

  // create socket
  socketFD = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
  if (socketFD < 0) {
    fprintf(stderr, "socket: %s (%d)\n", strerror(errno), errno);
    freeaddrinfo(res);
    return false;
  }

  // connect to server
  if (::connect(socketFD, res->ai_addr, res->ai_addrlen) == -1) {
    fprintf(stderr, "connect: %s (%d)\n", strerror(errno), errno);
    freeaddrinfo(res);
    return false;
  }
  freeaddrinfo(res);
  return true;
}

bool ThreadSafeSocket::write(const std::byte *data, const size_t dataSize) {
  std::unique_lock<std::mutex> lock(writeLock);
  if (send(socketFD, data, dataSize, 0) == -1) {
    fprintf(stderr, "send: %s (%d)\n", strerror(errno), errno);
    return false;
  }
  return true;
}

bool ThreadSafeSocket::writeHeaderAndData(const std::byte header[6], const std::byte *data, size_t dataSize) {
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
    // should not be possible to reach here, but just in case...
    std::cerr << "Error reading from socket. Amount read does not match amount requested to read\n";
    exit(1);
  }
  return bufferSize;
}

int ThreadSafeSocket::getSocketFD() const {
  return socketFD;
}
