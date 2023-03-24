/**
 * @author Justin Nicolas Allard
 * Implementation file for socket class
*/

#include <string>
#include <vector>
#include <cstring>
#include <iostream>
#include <fstream>
#include <netdb.h>
#include <unistd.h>
#include <sys/socket.h>

#include "BaseSocket.hpp"

BaseSocket::BaseSocket(const int socketFD):
  socketFD{socketFD} {}

BaseSocket::~BaseSocket() {
  if (socketFD != 0) {
    close(socketFD);
  }
}

BaseSocket::BaseSocket(BaseSocket &&moved): socketFD{moved.socketFD} {
  moved.socketFD = 0;
}

BaseSocket::operator int() {
  return socketFD;
}

void BaseSocket::setSocketFD(int newFD) {
  socketFD = newFD;
}

bool BaseSocket::connect(const std::string &ip, const uint16_t port) {
  struct addrinfo hints;
  struct addrinfo *res;  // will point to the results
  memset(&hints, 0, sizeof hints); // make sure the struct is empty
  hints.ai_family = AF_UNSPEC;     // don't care IPv4 or IPv6
  hints.ai_socktype = SOCK_STREAM; // TCP stream sockets
  // get ready to connect
  if (getaddrinfo(ip.c_str(), std::to_string(port).c_str(), &hints, &res) == -1) {
    fprintf(stderr, "getaddrinfo: %s (%d)\n", strerror(errno), errno);
    return false;
  };

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

bool BaseSocket::write(const std::byte *data, const size_t dataSize) const {
  ssize_t numSent = 0;
  if ((numSent = send(socketFD, data, dataSize, 0)) == -1) {
    fprintf(stderr, "send: %s (%d)\n", strerror(errno), errno);
    return false;
  }
  return true;
}

size_t BaseSocket::read(std::byte *buffer, const size_t bufferSize) {
  const ssize_t numReadBytes = recv(socketFD, buffer, bufferSize, 0);
  if (numReadBytes == -1) {
    fprintf(stderr, "recv: %s (%d)\n", strerror(errno), errno);
    exit(1);
  }
  return static_cast<size_t>(numReadBytes);
}

size_t BaseSocket::readAll(std::byte *buffer, const size_t bufferSize) {
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

bool BaseSocket::bind(const std::string &host, const uint16_t port) {
  // first, load up address structs with getaddrinfo():
  struct addrinfo hints;
  struct addrinfo *res;
  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_UNSPEC; // use IPv4 or IPv6, whichever
  hints.ai_socktype = SOCK_STREAM;
  if (getaddrinfo(host.c_str(), std::to_string(port).c_str(), &hints, &res) == -1) {
    fprintf(stderr, "getaddrinfo: %s (%d)\n", strerror(errno), errno);
    return false;
  };

  if (res == nullptr) {
    fprintf(stderr, "no results for \"%s\"\n", host.c_str());
    return false;
  }
  // make a socket:
  socketFD = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
  if (socketFD == -1) {
    fprintf(stderr, "socket: %s (%d)\n", strerror(errno), errno);
    freeaddrinfo(res);
    return false;
  }
  // set socket to immediately reuse port when the application closes
  const int reuse = 1;
  if (setsockopt(socketFD, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) == -1) {
    fprintf(stderr, "setsockopt: %s (%d)\n", strerror(errno), errno);
    freeaddrinfo(res);
    return false;
  }

  if (::bind(socketFD, res->ai_addr, res->ai_addrlen) == -1) {
    fprintf(stderr, "bind: %s (%d)\n", strerror(errno), errno);
    freeaddrinfo(res);
    return false;
  }
  freeaddrinfo(res);
  return true;
}

bool BaseSocket::listen(int backlog) {
  /* Set a default value if the backlog is negative */
  if (backlog < 0)
    backlog = 20;

  if (::listen(socketFD, backlog) == -1) {
    fprintf(stderr, "listen: %s (%d)\n", strerror(errno), errno);
    return false;
  }
  return true;
}

BaseSocket BaseSocket::accept() {
  struct sockaddr serverAddr; // not used at the moment
  socklen_t sockLen = sizeof (serverAddr);
  const int clientFD = ::accept(socketFD, &serverAddr, &sockLen); 
  if (clientFD == -1) {
    fprintf(stderr, "accept: %s (%d)\n", strerror(errno), errno);
  }
  return BaseSocket(clientFD);
}

int BaseSocket::getSocketFD() const {
  return socketFD;
}
