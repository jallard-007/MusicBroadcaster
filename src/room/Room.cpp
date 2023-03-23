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

  // set initial max file descriptor for selector
  fdMax = hostSocket.getSocketFD();

  FD_ZERO(&master);    // clear the master sets
  // add stdin and the hostSocket to selector list
  FD_SET(0, &master);
  FD_SET(hostSocket, &master);

  manageRoom();
} 

void Room::manageRoom() {
  while (1) {
    fd_set read_fds = master;  // temp file descriptor list for select()
    if (::select(fdMax + 1, &read_fds, nullptr, nullptr, nullptr /* <- time out in microseconds*/) == -1){
      fprintf(stderr, "select: %s (%d)\n", strerror(errno), errno);
      exit(1);
    }

    if (FD_ISSET(hostSocket, &read_fds)) { // connection request, add them to the room
      _handleConnectionRequests();
    }
    if (FD_ISSET(0, &read_fds)) { // input from stdin, local user entered a command
      // handle command line input here
      std::string input;
      std::getline(std::cin, input);
      if (input == "exit") {
        return;
      }
    }

    // loop through all clients to see if they sent something
    std::list<room::Client>::iterator client = clients.begin();
    while (client != clients.end()) {
      if (FD_ISSET(client->getSocket(), &read_fds)) { // check if file descriptor is set, if it is, that means theres something to read
        // request from a client
        std::byte requestHeader[6];
        size_t numBytesRead = 0;
        // if for some reason the call to `read` gets less than 6 bytes, this loop is here to insure we read all 6
        while (numBytesRead < 6) {
          numBytesRead += client->getSocket().read(requestHeader + numBytesRead, sizeof requestHeader - numBytesRead);
          if (numBytesRead == 0) {
            // connection was closed, remove the client.
            std::cout << "A client disconnected\n";
            FD_CLR(client->getSocket(), &master);
            client = clients.erase(client);
            break;
          } else if (numBytesRead == 6){
            _handleClientRequest(*client, requestHeader);
            ++client;
            break;
          }
        }
      } else { // nothing from this client
        // increment to next client
        ++client;
      }
    }
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

void Room::_handleConnectionRequests() {
  BaseSocket clientSocket = hostSocket.accept();
  if (clientSocket == -1) {
    // error accepting connection, skip
    return;
  }
  if (clientSocket >= fdMax) {
    // new fileDescriptor is greater than previous greatest, update it
    fdMax = clientSocket;
  }
  std::cout << "Client connected\n";
  // finally, add the client to both the selector list and the room list
  FD_SET(clientSocket, &master);
  this->addClient({"user", std::move(clientSocket)});
}

void Room::_handleClientRequest(room::Client &client, const std::byte *requestHeader) {
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
}

void Room::_attemptAddSongToQueue(room::Client& client, const size_t sizeOfFile) {
  // atomic bool here again
  while (1) {
    BaseSocket &socket = client.getSocket();

    // add a Music object into the queue
    Music *queueEntry = queue.add();

    if (queueEntry == nullptr) {
      // adding to queue was unsuccessful, we can release the lock right away
      // send a message back to client to deny their request to add a song
      _sendBasicResponse(socket, Command::RES_NOT_OK);
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

    break;
  }
}

void Room::_sendBasicResponse(BaseSocket& socket, Command response) {
  std::byte responseHeader[6];
  ::memset(responseHeader, 0, sizeof responseHeader);
  responseHeader[0] = static_cast<std::byte>(response);
  socket.write(responseHeader, sizeof responseHeader);
}
