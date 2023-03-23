/**
 * \author Justin Nicolas Allard
 * Implementation file for server side client class
*/

#include <thread>
#include "../socket/BaseSocket.hpp"
#include "Client.hpp"

room::Client::Client(): name{}, socket{}, thread{} {}

room::Client::Client(const std::string &name, int socketFD):
  name{name}, socket{socketFD}, thread{} {}

room::Client::Client(Client &&moved): name{std::move(moved.name)}, socket{moved.socket}, thread{} {
  moved.socket.setSocketFD(0);
}

const std::string &room::Client::getName() const {
  return name;
};

const std::thread &room::Client::getThread() const {
  return thread;
}

void room::Client::setThread(std::thread &&movedThread) {
  thread = std::move(movedThread);
}

void room::Client::waitForThread() {
  if (thread.joinable()) {
    thread.join();
  }
}

BaseSocket &room::Client::getSocket() {
  return socket;
};
