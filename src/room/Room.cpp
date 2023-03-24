/**
 * @author Justin Nicolas Allard
 * Implementation file for room class
*/

#include <iostream>
#include <vector>
#include <chrono>
#include <cstdlib>
#include <cstring>
#include <sys/select.h>
#include <unistd.h>
#include <thread>

#include "Room.hpp"
#include "Client.hpp"
#include "../CLInput.hpp"
#include "../socket/BaseSocket.hpp"
#include "../music/Player.hpp"
#include "../messaging/Message.hpp"
#include "../messaging/Commands.hpp"

using namespace Commands;

Room::Room(): ip{0}, fdMax{0}, hostSocket{0},
  threadPipe{0}, queueMutex{}, name{"unnamed"},
  clients{}, queue{}, master{}
  {}

Room::~Room() {
  if (threadPipe[0] != 0) {
    close(threadPipe[0]);
  }
  if (threadPipe[1] != 0) {
    close(threadPipe[1]);
  }
}

bool Room::initializeRoom() {
  const uint16_t port = getPort();
  std::string host;
  getHost(host);
  if (!hostSocket.bind(host, port)) {
    return false;
  }
  if (!hostSocket.listen()) {
    return false;
  }

  // create pipe for thread communication
  if (::pipe(threadPipe) == -1) {
    fprintf(stderr, "pipe: %s (%d)\n", strerror(errno), errno);
    return false;
  }

  // set initial max file descriptor for selector
  fdMax = hostSocket.getSocketFD() > threadPipe[0] ? hostSocket.getSocketFD() : threadPipe[0];

  // clear the master sets
  FD_ZERO(&master);
  // add stdin, the hostSocket, and the pipe to selector list
  FD_SET(0, &master);
  FD_SET(hostSocket, &master);
  FD_SET(threadPipe[0], &master);
  return true;
}

bool Room::launchRoom() {
  while (1) {
    fd_set read_fds = master;  // temp file descriptor list for select()
    if (::select(fdMax + 1, &read_fds, nullptr, nullptr, nullptr /* <- time out in microseconds*/) == -1){
      fprintf(stderr, "select: %s (%d)\n", strerror(errno), errno);
      return false;
    }

    // connection request, add them to the room
    if (FD_ISSET(hostSocket, &read_fds)) { 
      _handleConnectionRequests();
    }

    // input from stdin, local user entered a command
    if (FD_ISSET(0, &read_fds)) { 
      // handle command line input here
      std::string input;
      std::getline(std::cin, input);
      if (input == "exit") {
        return true;
      }
    }

    // data from pipe, a thread has finished
    if (FD_ISSET(threadPipe[0], &read_fds)) { 
      /*
        When we get here, it means that a thread that was receiving an audio file has finished.
        So a song was added to the queue. Meaning that if the queue was empty, that song should play.
        Therefore we should send it to every client so that they can play it.
      */
      
      // read the data from the pipe, just one int. 
      // represents the fileDescriptor of the socket that was receiving an audio file
      int fdOfClientSocket = 0;
      ::read(threadPipe[0], reinterpret_cast<void *>(&fdOfClientSocket), sizeof (int));

      if (fdOfClientSocket < 0) { // when true, means that we need to remove that client
        fdOfClientSocket *= -1;
        // remove it
        clients.remove_if([fdOfClientSocket](room::Client &client){
          return client.getSocket().getSocketFD() == fdOfClientSocket;
        });
      } else { // other wise its all good, continue
        // add the client's socket FD back to select
        FD_SET(fdOfClientSocket, &master);
        
        // now, we can send the audio to all clients.
        // or if a song is already playing, maybe do it later

      }
    }

    // loop through all clients to see if they sent something
    std::list<room::Client>::iterator client = clients.begin();
    while (client != clients.end()) {
      if (FD_ISSET(client->getSocket(), &read_fds)) { // check if file descriptor is set, if it is, that means theres something to read
        // request from a client
        std::byte requestHeader[6];
        const size_t numBytesRead = client->getSocket().readAll(requestHeader, sizeof requestHeader);
        if (numBytesRead == 0) {
          std::cout << "A client disconnected\n";
          
          // connection was closed, remove the client.
          // we have to remove it from both the master list and the clients list
          FD_CLR(client->getSocket(), &master);
          clients.erase(client);
        } else {
          _handleClientRequest(*client, requestHeader);
        }
      }
      ++client;
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
    case Command::REQ_ADD_TO_QUEUE: {
      // remove the socket from the master list since we want only the child thread to read from it.
      // we add it back once the thread is finished executing
      FD_CLR(client.getSocket(), &master);
      std::thread clientThread = std::thread(
        &Room::_attemptAddSongToQueue,
        this,
        &client
      );
      clientThread.detach();
      break;
    }
    default:
      _sendBasicResponse(client.getSocket(), Command::BAD_VALUES);
      break;
  }
}

void *Room::_attemptAddSongToQueue(void *arg) {
  room::Client &client = *(static_cast<room::Client *>(arg));
  BaseSocket &socket = client.getSocket();

  // wait till we acquire the lock
  std::unique_lock<std::mutex> lock(queueMutex);

  // add a Music object into the queue
  Music *queueEntry = queue.add();

  if (queueEntry == nullptr) {
    // adding to queue was unsuccessful
    // send a message back to client to deny their request to add a song
    _sendBasicResponse(socket, Command::RES_NOT_OK);

    // notify parent thread to add this socket back in select
    const int socketFD = socket.getSocketFD();
    ::write(threadPipe[1], reinterpret_cast<const void *>(&socketFD), sizeof (int));
    return nullptr;
  }
  // adding to the queue was successful
  // send a message back to client to confirm that they can continue to send the song
  _sendBasicResponse(socket, Command::RES_OK);

  // read in the header, create message object from it
  std::byte requestHeader[6];
  socket.readAll(requestHeader, sizeof requestHeader);
  const Message message(requestHeader);

  // get size of message from message object
  const unsigned int sizeOfFile = message.getBodySize();

  // allocate memory for the file
  queueEntry->getBytes().resize(sizeOfFile); 

  std::byte *dataPointer = queueEntry->getBytes().data();

  int socketFD = socket.getSocketFD();
  // read from the socket
  const unsigned int numBytesRead = static_cast<unsigned int>(socket.readAll(dataPointer, sizeOfFile));
  if (numBytesRead <= 0) {
    // either client disconnected half way through, or some other error. Scrap it
    queue.removeByAddress(queueEntry);
    // make FD negative to tell parent thread we need to remove the client
    socketFD *= -1;
  }
  // notify parent thread that this thread is done
  ::write(threadPipe[1], reinterpret_cast<const void *>(&socketFD), sizeof (int));
  return nullptr;
}

void Room::_sendBasicResponse(BaseSocket& socket, Command response) {
  std::byte responseHeader[6];
  ::memset(responseHeader, 0, sizeof responseHeader);
  responseHeader[0] = static_cast<std::byte>(response);
  socket.write(responseHeader, sizeof responseHeader);
}
