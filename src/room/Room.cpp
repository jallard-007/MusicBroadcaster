/**
 * @author Justin Nicolas Allard
 * Implementation file for room class
*/

#include <iostream>
#include <vector>
#include <cstdlib>
#include <cstring>
#include <sys/select.h>
#include <unistd.h>
#include <thread>

#include "Room.hpp"
#include "Client.hpp"
#include "../CLInput.hpp"

using namespace Commands;

Room::Room(): ip{0}, fdMax{0}, hostSocket{0},
  threadRecvPipe{0}, threadSendPipe{0},
  queueMutex{}, name{"unnamed"},
  clients{}, queue{}, master{} {}

Room::~Room() {
  if (threadRecvPipe[0] != 0) {
    close(threadRecvPipe[0]);
  }
  if (threadRecvPipe[1] != 0) {
    close(threadRecvPipe[1]);
  }
  if (threadSendPipe[0] != 0) {
    close(threadSendPipe[0]);
  }
  if (threadSendPipe[1] != 0) {
    close(threadSendPipe[1]);
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
  if (::pipe(threadRecvPipe) == -1) {
    fprintf(stderr, "pipe: %s (%d)\n", strerror(errno), errno);
    return false;
  }
  if (::pipe(threadSendPipe) == -1) {
    fprintf(stderr, "pipe: %s (%d)\n", strerror(errno), errno);
    return false;
  }

  // set initial max file descriptor for selector
  fdMax = hostSocket.getSocketFD();
  fdMax = fdMax > threadRecvPipe[0] ? fdMax : threadRecvPipe[0];
  fdMax = fdMax > threadSendPipe[0] ? fdMax : threadSendPipe[0];

  // clear the master sets
  FD_ZERO(&master);
  // add stdin, the hostSocket, and the pipe to selector list
  FD_SET(0, &master);
  FD_SET(hostSocket.getSocketFD(), &master);
  FD_SET(threadRecvPipe[0], &master);
  FD_SET(threadSendPipe[0], &master);
  return true;
}

void Room::launchRoom() {
  while (true) {
    fd_set read_fds = master;  // temp file descriptor list for select()
    if (::select(fdMax + 1, &read_fds, nullptr, nullptr, nullptr /* <- time out in microseconds*/) == -1){
      fprintf(stderr, "select: %s (%d)\n", strerror(errno), errno);
      return;
    }

    // connection request, add them to the room
    if (FD_ISSET(hostSocket.getSocketFD(), &read_fds)) { 
      handleConnectionRequests();
    }

    // input from stdin, local user entered a command
    if (FD_ISSET(0, &read_fds)) { 
      if (!handleStdinCommands()) {
        return;
      }
    }

    // data from pipe, a thread has finished receiving an audio file
    if (FD_ISSET(threadRecvPipe[0], &read_fds)) { 
      processThreadFinishedReceiving();
    }

    // data from pipe, a thread as finished sending an audio file
    if (FD_ISSET(threadSendPipe[0], &read_fds)) {
      processThreadFinishedSending();
    }

    // loop through all clients to see if they sent something
    auto client = clients.begin();
    while (client != clients.end()) {
      // request from a client
      if (FD_ISSET(client->getSocket().getSocketFD(), &read_fds)) {
        if (!handleClientRequest(*client)) {
          // connection was closed, remove the client.
          // we have to remove it from both the master list and the clients list
          std::cout << "A client disconnected\n";
          FD_CLR(client->getSocket().getSocketFD(), &master);
          client = clients.erase(client);
        } else {
          ++client;
        }
      } else {
        ++client;
      }
    }
  }
}

void Room::handleConnectionRequests() {
  ThreadSafeSocket clientSocket{hostSocket.accept()};
  if (clientSocket.getSocketFD() == -1) {
    // error accepting connection, skip
    return;
  }
  if (clientSocket.getSocketFD() >= fdMax) {
    // new fileDescriptor is greater than previous greatest, update it
    fdMax = clientSocket.getSocketFD();
  }
  std::cout << "Client connected\n";
  // finally, add the client to both the selector list and the room list
  FD_SET(clientSocket.getSocketFD(), &master);
  this->addClient({"user", std::move(clientSocket)});
}

bool Room::handleStdinCommands() {
  std::string input;
  std::getline(std::cin, input);
  if (input == "exit") {
    return false;
  }
  return true;
}

void Room::processThreadFinishedReceiving() {
  int fdOfClientSocket = 0;
  ::read(threadRecvPipe[0], reinterpret_cast<void *>(&fdOfClientSocket), sizeof (int));

  if (fdOfClientSocket < 0) { // when true, means that we need to remove that client
    fdOfClientSocket *= -1;
    std::cout << "A client disconnected\n";
    // remove it
    clients.remove_if([fdOfClientSocket](room::Client &client){
      return client.getSocket().getSocketFD() == fdOfClientSocket;
    });
  } else { // other wise its all good, continue
    sendSongToAllClients();
  }
}

void Room::processThreadFinishedSending() {
  int fdOfClientSocket = 0;
  ::read(threadSendPipe[0], reinterpret_cast<void *>(&fdOfClientSocket), sizeof (int));
  if (fdOfClientSocket < 0) { // when true, means that we need to remove that client
    fdOfClientSocket *= -1;
    std::cout << "A client disconnected\n";
    // remove it
    clients.remove_if([fdOfClientSocket](room::Client &client){
      return client.getSocket().getSocketFD() == fdOfClientSocket;
    });
  } else { // other wise its all good, continue
    // add the client's socket FD back to select
    FD_SET(fdOfClientSocket, &master);
  }
}

bool Room::handleClientRequest(room::Client &client) {
  std::byte requestHeader[6];
  const size_t numBytesRead = client.getSocket().readAll(requestHeader, sizeof requestHeader);
  if (numBytesRead == 0) {
    return false;
  }
  // convert header to message
  Message message(requestHeader);
  // handle every supported message here
  auto command = static_cast<Command>(message.getCommand());
  switch(command) {
    case Command::REQ_ADD_TO_QUEUE: {
      // remove the socket from the master list since we want only the child thread to read from it.
      // we add it back once the thread is finished executing
      FD_CLR(client.getSocket().getSocketFD(), &master);
      std::thread clientThread = std::thread(
        &Room::attemptAddSongToQueue,
        this,
        &client
      );
      clientThread.detach();
      break;
    }
    default:
      sendBasicResponse(client.getSocket(), Command::BAD_VALUES);
      break;
  }
  return true;
}

void Room::sendSongDataToClient(std::shared_ptr<Music> audio, room::Client &client) {
  Message message;
  message.setCommand(static_cast<std::byte>(Command::SONG_DATA));
  message.setBodySize(static_cast<uint32_t>(audio.get()->getBytes().size()));
  int socketFD = client.getSocket().getSocketFD();
  if (!client.getSocket().writeHeaderAndData(message.format().data(), audio.get()->getBytes().data(), audio.get()->getBytes().size())) {
    socketFD *= -1;
  }
  ::write(threadSendPipe[1], reinterpret_cast<const void *>(&socketFD), sizeof (int));
}

void Room::sendSongToAllClients() {
  auto next = queue.getFrontMem();
  if (next != nullptr) {
    for (room::Client &client : clients) {
      FD_CLR(client.getSocket().getSocketFD(), &master);
      std::thread thread = std::thread(&Room::sendSongDataToClient, this, next, std::ref(client));
      thread.detach();
    }
  }
}

void Room::attemptAddSongToQueue(room::Client* clientPtr) {
  room::Client &client = *clientPtr;
  ThreadSafeSocket &socket = client.getSocket();
  int socketFD = socket.getSocketFD();

  auto process = [this, &socket, &socketFD]() {
    // wait till we acquire the lock
    std::unique_lock<std::mutex> lock{queueMutex};
    // add a Music object into the queue
    FILE *queueEntry = queue.add();

    if (queueEntry == nullptr) {
      // adding to queue was unsuccessful
      // send a message back to client to deny their request to add a song
      sendBasicResponse(socket, Command::RES_NOT_OK);
      return;
    }
    // adding to the queue was successful
    // send a message back to client to confirm that they can continue to send the song
    sendBasicResponse(socket, Command::RES_OK);
    // read in the header, create message object from it
    std::byte requestHeader[6];
    {
      const auto numBytesRead = static_cast<uint32_t>(socket.readAll(requestHeader,  sizeof requestHeader));
      if (numBytesRead <= 0) {
        // either client disconnected half way through, or some other error. Scrap it
        queue.removeByAddress(queueEntry);
        // make FD negative to tell parent thread we need to remove the client
        socketFD *= -1;
        return;
      }
    }
    const Message message(requestHeader);

    // get size of message from message object
    const uint32_t sizeOfFile = message.getBodySize();
    Music music;
    music.getBytes().resize(sizeOfFile); 

    std::byte *dataPointer = music.getBytes().data();

    const auto numBytesRead = static_cast<uint32_t>(socket.readAll(dataPointer, sizeOfFile));
    if (numBytesRead <= 0) {
      // either client disconnected half way through, or some other error. Scrap it
      queue.removeByAddress(queueEntry);
      // make FD negative to tell parent thread we need to remove the client
      socketFD *= -1;
    }
    music.writeToFile(queueEntry);
  };

  process();

  // notify parent thread that this thread is done
  ::write(threadRecvPipe[1], reinterpret_cast<const void *>(&socketFD), sizeof (int));
}

void Room::sendBasicResponse(ThreadSafeSocket& socket, Command response) {
  std::byte responseHeader[6];
  ::memset(responseHeader, 0, sizeof responseHeader);
  responseHeader[0] = static_cast<std::byte>(response);
  socket.write(responseHeader, sizeof responseHeader);
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
