/**
 * @author Justin Nicolas Allard
 * Implementation file for server side client class
*/

#include <thread>
#include "../socket/ThreadSafeSocket.hpp"
#include "Client.hpp"

room::Client::Client(): name{}, socket{} {}

room::Client::Client(const std::string &name, ThreadSafeSocket &&socket):
  name{name}, socket{std::move(socket)} {}

room::Client::Client(Client &&moved):
  name{std::move(moved.name)}, socket{std::move(moved.socket)} {}

bool room::Client::operator==(const room::Client &rhs) const {
  return rhs.socket.getSocketFD() == socket.getSocketFD();
}

const std::string &room::Client::getName() const {
  return name;
}

ThreadSafeSocket &room::Client::getSocket() {
  return socket;
}
