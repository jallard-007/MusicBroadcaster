/**
 * @author Justin Nicolas Allard
 * Implementation file for server side client class
*/

#include <utility>

#include "Client.hpp"

room::Client::Client(): entriesTillSynced{0}, p_entry{}, name{}, socket{} {}

room::Client::Client(std::string name, ThreadSafeSocket &&socket):
  entriesTillSynced{0}, p_entry{}, name{std::move(name)}, socket{std::move(socket)} {}

room::Client::Client(Client &&moved) noexcept:
  entriesTillSynced{moved.entriesTillSynced}, name{std::move(moved.name)}, socket{std::move(moved.socket)} {}

bool room::Client::operator==(const room::Client &rhs) const {
  return rhs.socket.getSocketFD() == socket.getSocketFD();
}

const std::string &room::Client::getName() const {
  return name;
}

ThreadSafeSocket &room::Client::getSocket() {
  return socket;
}
