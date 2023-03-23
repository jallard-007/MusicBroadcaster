/**
 * \author Justin Nicolas Allard
 * Implementation file for room class
*/

#include <iostream>
#include <vector>
#include <chrono>
#include <cstdlib>
#include <cstring>

#include "Room.hpp"
#include "Client.hpp"
#include "../CLInput.hpp"
#include "../socket/BaseSocket.hpp"
#include "../music/Player.hpp"
#include "../messaging/Message.hpp"
#include "../messaging/Commands.hpp"

using namespace Commands;

Room::Room() {

  const uint16_t port = getPort();
  std::string host;
  getHost(host);
  hostSocket.bind(host, port);
  hostSocket.listen();

  // ask for room name on command line here to register with tracker

  // launch accept thread, this will accept incoming connections
  // each connection will also launch a thread to listen to that client
  acceptThread = std::thread(&Room::_handleConnectionRequests, this);

  // we could do other things here such as sending file to clients
  // use signalling to tell main thread what to do

  // wait on accept thread
  if (acceptThread.joinable()) {
    acceptThread.join();
  }
}

void Room::setIp(int newIp) {
  ip = newIp;
}

void Room::printClients() {
  std::cout << getName() << " clients:\n";
  for (room::Client &client : clients) {
    std::cout << client.getName() << " " << client.getSocket().getSocketFD() << '\n';
  }
}

room::Client &Room::addClient(room::Client &&newClient) {
  room::Client &client = clients.emplace_back(std::move(newClient));
  // launch client thread to listen for messages
  client.setThread(std::thread(&Room::_handleClientSocket, this, &client));
  return client;
}

const room::Client *Room::searchClient(int client_id) {
  for (room::Client &client : clients) {
    if (client.getSocket().getSocketFD() == client_id) {
      return &client;
    }
  }
  return nullptr;
}

MusicStorage &Room::getQueue() {
  return queue;
}

int Room::getIp() const {
  return ip;
}

const std::string & Room::getName() const  {
  return name;
}

const std::list<room::Client> &Room::getClients() const {
  return clients;
}

/**
 * The below functions deal with handling requests from connected clients
*/

void *Room::_handleConnectionRequests() {
  std::cout << "Listening for connection requests...\n";
  // use atomic bool here to signal stop
  while (1) {
    // accept connection and add client to room's client list
    const int socketFD = hostSocket.accept();
    std::cout << "Client connected\n";
    this->addClient({"user", socketFD});
  }
}

void *Room::_handleClientSocket(void *arg) {
  // cast argument into a room::Client object
  room::Client &client = *(static_cast<room::Client *>(arg));

  do {
    // read in bytes corresponding to size of the header, see Message class
    std::byte requestHeader[6];
    const size_t numBytesRead = client.getSocket().read(requestHeader, sizeof requestHeader);
    if (numBytesRead == 0) {
      // connection was probably closed.
      std::cout << "Client disconnected\n";
      return nullptr;
    }

    // convert header to message
    Message message(requestHeader);

    // handle every supported message here
    Command command = static_cast<Command>(message.getCommand());
    switch(command) {
      case Command::REQ_ADD_TO_QUEUE:
        _attemptAddSongToQueue(client, message.getBodySize());
        break;
      default:
        _sendBasicResponse(client.getSocket(), Command::BAD_VALUES);
        break;
    }
  } while (1); // NOTE: should use an atomic bool here to communicate between threads
  return nullptr;
}

void Room::_attemptAddSongToQueue(room::Client& client, const size_t sizeOfFile) {
  // atomic bool here again
  while (1) {
    if (!queueMutex.try_lock()) {
      // we could not lock, try again later

      // how long to wait till we try to acquire lock again
      static const std::chrono::milliseconds interval(100);
      std::this_thread::sleep_for(interval);
      continue; // loop back
    }
    // we locked successfully

    BaseSocket &socket = client.getSocket();

    // add a Music object into the queue
    Music *queueEntry = queue.add();

    if (queueEntry == nullptr) {
      // adding to queue was unsuccessful, we can release the lock right away
      // send a message back to client to deny their request to add a song
      _sendBasicResponse(socket, Command::RES_NOT_OK);
      queueMutex.unlock(); // release lock
      break;
    }
    // adding to the queue was successful
    // send a message back to client to confirm that they can continue to send the song
    _sendBasicResponse(socket, Command::RES_OK);

    // allocate memory for the file
    queueEntry->getBytes().resize(sizeOfFile); 

    std::byte *dataPointer = queueEntry->getBytes().data();
    unsigned int totalNumBytesRead = 0;

    do {
      // read in the header, create message object from it
      std::byte requestHeader[6];
      socket.read(requestHeader, sizeof requestHeader);
      const Message message(requestHeader);

      // get size of message from message object
      const unsigned int amountToRead = message.getBodySize();

      // initialize counter for number of bytes read
      unsigned int numBytesRead = 0;
      do {
        // read from the socket
        numBytesRead += static_cast<unsigned int>(socket.read(dataPointer + numBytesRead, amountToRead - numBytesRead));

        // loop until we have the whole message
      } while (numBytesRead < amountToRead);

      // add body size to total
      totalNumBytesRead += numBytesRead;

      // loop until we have the whole file
    } while (totalNumBytesRead < sizeOfFile);

    // we have read the whole file, can release the lock
    // NOTE: we could release the lock just after we allocate memory and then use a lock on the Music object instead
    queueMutex.unlock(); // release lock
    break;
  }
}

void Room::_sendBasicResponse(BaseSocket& socket, Command response) {
  std::byte responseHeader[6];
  ::memset(responseHeader, 0, sizeof responseHeader);
  responseHeader[0] = static_cast<std::byte>(response);
  socket.write(responseHeader, sizeof responseHeader);
}
