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
#include <unordered_map>

#include "../debug.hpp"
#include "Room.hpp"
#include "Client.hpp"
#include "../CLInput.hpp"
#include "../messaging/Message.hpp"

using namespace Commands;

Room::Room(): ip{}, fdMax{}, numClientsReceivedFile{}, hostSocket{},
  threadRecvPipe{}, threadSendPipe{}, threadWaitAudioPipe{}, name{},
  clients{}, queue{}, audioPlayer{}, queueMutex{}, master{} {}

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
  if (threadWaitAudioPipe[0] != 0) {
    close(threadWaitAudioPipe[0]);
  }
  if (threadWaitAudioPipe[1] != 0) {
    close(threadWaitAudioPipe[1]);
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
  if (::pipe(threadWaitAudioPipe) == -1) {
    fprintf(stderr, "pipe: %s (%d)\n", strerror(errno), errno);
    return false;
  }

  // set initial max file descriptor for selector
  fdMax = hostSocket.getSocketFD();
  fdMax = fdMax > threadRecvPipe[0] ? fdMax : threadRecvPipe[0];
  fdMax = fdMax > threadSendPipe[0] ? fdMax : threadSendPipe[0];
  fdMax = fdMax > threadWaitAudioPipe[0] ? fdMax : threadWaitAudioPipe[0];

  // clear the master sets
  FD_ZERO(&master);
  // add stdin, the hostSocket, and the pipe to selector list
  FD_SET(0, &master);
  FD_SET(hostSocket.getSocketFD(), &master);
  FD_SET(threadRecvPipe[0], &master);
  FD_SET(threadSendPipe[0], &master);
  FD_SET(threadWaitAudioPipe[0], &master);
  std::cout << "Successfully created a room\n";
  return true;
}

void Room::launchRoom() {
  std::cout << " >> ";
  std::cout.flush();
  while (true) {
    fd_set read_fds = master;  // temp file descriptor list for select()
    if (::select(fdMax + 1, &read_fds, nullptr, nullptr, nullptr /* <- time out in microseconds*/) == -1){
      fprintf(stderr, "select: %s (%d)\n", strerror(errno), errno);
      return;
    }

    // connection request, add them to the room
    if (FD_ISSET(hostSocket.getSocketFD(), &read_fds)) {
      DEBUG_P(std::cout << "connection request\n");
      handleConnectionRequests();
    }

    // input from stdin, local user entered a command
    if (FD_ISSET(0, &read_fds)) { 
      DEBUG_P(std::cout << "stdin command entered\n");
      handleStdinCommands();
    }

    // data from pipe, a thread has finished receiving an audio file
    if (FD_ISSET(threadRecvPipe[0], &read_fds)) {
      DEBUG_P(std::cout << "data from recv pipe\n");
      processThreadFinishedReceiving();
    }

    // data from pipe, a thread as finished sending an audio file
    if (FD_ISSET(threadSendPipe[0], &read_fds)) {
      DEBUG_P(std::cout << "data from send pipe\n");
      processThreadFinishedSending();
    }

    if (FD_ISSET(threadWaitAudioPipe[0], &read_fds)) {
      DEBUG_P(std::cout << "data from song wait pipe\n");
      int x;
      ::read(threadWaitAudioPipe[0], reinterpret_cast<void *>(&x), sizeof (int));
      queue.removeFront();
      attemptPlayNext();
    }

    // loop through all clients to see if they sent something
    auto client = clients.begin();
    while (client != clients.end()) {
      // request from a client
      if (FD_ISSET(client->getSocket().getSocketFD(), &read_fds)) {
        DEBUG_P(std::cout << "data from client socket\n");
        if (!handleClientRequest(*client)) {
          DEBUG_P(std::cout << "client disconnected\n");
          // connection was closed, remove the client.
          // we have to remove it from both the master list and the clients list
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
  // finally, add the client to both the selector list and the room list
  FD_SET(clientSocket.getSocketFD(), &master);
  this->addClient({"user", std::move(clientSocket)});
}

enum class RoomCommand {
  FAQ,
  HELP,
  EXIT,
  ADD_SONG,
  SEEK,
  PLAY
};

const std::unordered_map<std::string, RoomCommand> roomCommandMap = {
  {"faq", RoomCommand::FAQ},
  {"help", RoomCommand::HELP},
  {"exit", RoomCommand::EXIT},
  {"add song", RoomCommand::ADD_SONG},
  {"seek", RoomCommand::SEEK},
  {"play", RoomCommand::PLAY}
};

void roomShowHelp() {
  std::cout <<
  "List of commands as room host:\n\n";
}

void roomShowFAQ() {
  std::cout <<
  "Question 1:\n\n";
}

void Room::handleStdinCommands() {
  std::string input;
  std::getline(std::cin, input);
  RoomCommand command;
  try {
    command = roomCommandMap.at(input);
  } catch (const std::out_of_range &err){
    std::cout << "Invalid command. Try 'help' for information\n";
    std::cout << " >> ";
    std::cout.flush();
    return;
  }

  switch (command) {
    case RoomCommand::FAQ:
      roomShowFAQ();
      break;

    case RoomCommand::HELP:
      roomShowHelp();
      break;

    case RoomCommand::EXIT:
      exit(0);

    case RoomCommand::ADD_SONG:
      break;

    case RoomCommand::PLAY:
      break;

    case RoomCommand::SEEK:
      break;

    default:
      // this section of code should never be reached
      std::cerr << "Error: Reached default case in Room::handleStdinCommands\nCommand " << input << " not handled but is in clientMapCommand\n";
      exit(1);
  }
  std::cout << " >> ";
  std::cout.flush();
}

void Room::processThreadFinishedReceiving() {
  struct {
    int socketFD;
    MusicStorageEntry *p_queue;
  } t;
  ::read(threadRecvPipe[0], reinterpret_cast<void *>(&t), sizeof t);

  if (t.socketFD < 0) { // when true, means that we need to remove that client
    t.socketFD *= -1;
    // remove it
    clients.remove_if([&t](room::Client &client){
      return client.getSocket().getSocketFD() == t.socketFD;
    });
  } else { // other wise its all good, continue
    sendSongToAllClients(t.p_queue);
  }
}

void Room::processThreadFinishedSending() {
  int fdOfClientSocket = 0;
  ::read(threadSendPipe[0], reinterpret_cast<void *>(&fdOfClientSocket), sizeof (int));
  if (fdOfClientSocket < 0) { // when true, means that we need to remove that client
    fdOfClientSocket *= -1;
    // remove it
    clients.remove_if([fdOfClientSocket](room::Client &client){
      return client.getSocket().getSocketFD() == fdOfClientSocket;
    });
  } else { // other wise its all good, continue
    // add the client's socket FD back to select
    FD_SET(fdOfClientSocket, &master);
    ++numClientsReceivedFile;
    attemptPlayNext();
  }
}

void Room::waitOnAudio() {
  DEBUG_P(std::cout << "waiting for audio to finish\n");
  audioPlayer.wait();
  DEBUG_P(std::cout << "audio finished\n");
  int nothing;
  write(threadWaitAudioPipe[1], &nothing, sizeof nothing);
}

void Room::attemptPlayNext() {
  DEBUG_P(std::cout << "attempt play next\n");
  if (audioPlayer.isPlaying()) {
    DEBUG_P(std::cout << "audio still playing, cancel\n");
    return;
  }

  auto musicEntry = queue.getFront();
  if (musicEntry == nullptr) {
    DEBUG_P(std::cout << "next in queue is empty\n");
    return;
  } else if (!musicEntry->entryMutex.try_lock()) {
    DEBUG_P(std::cout << "could not get queue entry mutex, cancelling\n");
    return;
  }
  DEBUG_P(std::cout << "got queue entry mutex\n");
  DEBUG_P(std::cout << "sending play next message to all clients\n");
  for (room::Client &client : clients) {
    Message message;
    message.setCommand(Command::PLAY_NEXT);
    // might want to thread this off, similar to sendSongDataToClient
    client.getSocket().write(message.data(), message.size());
  }
  DEBUG_P(std::cout << "feeding next in queue to audioPlayer\n");
  audioPlayer.feed(musicEntry->path.c_str());
  audioPlayer.play();
  std::thread threadAudioWait = std::thread(&Room::waitOnAudio, this);
  threadAudioWait.detach();
  musicEntry->entryMutex.unlock();
  DEBUG_P(std::cout << "unlocked queue entry mutex\n");
}

bool Room::handleClientRequest(room::Client &client) {
  std::byte requestHeader[6];
  const size_t numBytesRead = client.getSocket().readAll(requestHeader, sizeof requestHeader);
  if (numBytesRead == 0) {
    return false;
  }
  DEBUG_P(std::cout << "read client request\n");
  // convert header to message
  Message message(requestHeader);
  // handle every supported message here
  auto command = static_cast<Command>(message.getCommand());
  switch(command) {
    case Command::REQ_ADD_TO_QUEUE: {
      DEBUG_P(std::cout << "req add to queue request\n");
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
      DEBUG_P(std::cout << "bad request\n");
      sendBasicResponse(client.getSocket(), Command::BAD_VALUES);
      break;
  }
  return true;
}

void Room::sendSongDataToClient(std::shared_ptr<Music> audio, room::Client &client) {
  int socketFD = client.getSocket().getSocketFD();

  auto process = [&socketFD, &client, &audio]() {
    ThreadSafeSocket &clientSocket = client.getSocket();
    { // send file to client
      const auto &audioData = audio.get()->getVector();
      Message message;
      message.setCommand(static_cast<std::byte>(Command::SONG_DATA));
      message.setBodySize(static_cast<uint32_t>(audioData.size()));
      DEBUG_P(std::cout << "sending file to client\n");
      if (!clientSocket.writeHeaderAndData(message.data(), audioData.data(), audioData.size())) {
        socketFD *= -1;
        return;
      }
    }
    { // wait for ok response from client
      std::byte responseHeader[SIZE_OF_HEADER];
      ::memset(responseHeader, 0, sizeof responseHeader);
      DEBUG_P(std::cout << "waiting for ok from client\n");
      if (clientSocket.readAll(responseHeader, sizeof responseHeader) == 0) {
        socketFD *= -1;
        return;
      }
      Message message(responseHeader);
      if (static_cast<Command>(message.getCommand()) != Command::RECV_OK) {
        socketFD *= -1;
        DEBUG_P(std::cout << "got something other than ok from client\n");
        return;
      }
      DEBUG_P(std::cout << "got ok from client\n");

    }
  };

  process();
  ::write(threadSendPipe[1], reinterpret_cast<const void *>(&socketFD), sizeof (int));
}

void Room::sendSongToAllClients(MusicStorageEntry *next) {
  DEBUG_P(std::cout << "sending next song to all clients\n");
  if (next == nullptr) {
    DEBUG_P(std::cout << "next is nullptr\n");
    return;
  }
  if (!next->entryMutex.try_lock()) {
    return;
  }
  Music m;
  m.setPath(next->path);
  auto data = m.getMemShared();
  for (room::Client &client : clients) {
    FD_CLR(client.getSocket().getSocketFD(), &master);
    std::thread thread = std::thread(&Room::sendSongDataToClient, this, data, std::ref(client));
    thread.detach();
  }
  next->entryMutex.unlock();
}

void Room::attemptAddSongToQueue(room::Client* clientPtr) {
  room::Client &client = *clientPtr;
  ThreadSafeSocket &socket = client.getSocket();
  int socketFD = socket.getSocketFD();
  MusicStorageEntry *queueEntry;

  auto process = [this, &socket, &socketFD, &queueEntry]() {
    // wait till we acquire the lock
    DEBUG_P(std::cout << "waiting for queue mutex\n");
    std::unique_lock<std::mutex> lock{queueMutex};
    // add a Music object into the queue
    DEBUG_P(std::cout << "got queue mutex\n");
    queueEntry = queue.add();
    DEBUG_P(std::cout << "waiting for queueEntry mutex\n");
    std::unique_lock<std::mutex> entryLock{queueEntry->entryMutex};
    DEBUG_P(std::cout << "got queueEntry mutex\n");
    lock.unlock();
    DEBUG_P(std::cout << "unlock queue mutex\n");
    if (queueEntry == nullptr) {
      // adding to queue was unsuccessful
      // send a message back to client to deny their request to add a song
      sendBasicResponse(socket, Command::RES_NOT_OK);
      DEBUG_P(std::cout << "res not ok, no room in queue\n");
      return;
    }
    // adding to the queue was successful
    // send a message back to client to confirm that they can continue to send the song
    sendBasicResponse(socket, Command::RES_OK);
    DEBUG_P(std::cout << "res ok, now waiting for song\n");
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
    DEBUG_P(std::cout << "got header, now reading in file of size " << sizeOfFile << "\n");
    Music music;
    music.getVector().resize(sizeOfFile); 

    std::byte *dataPointer = music.getVector().data();
    const auto numBytesRead = static_cast<uint32_t>(socket.readAll(dataPointer, sizeOfFile));
    if (numBytesRead <= 0) {
      // either client disconnected half way through, or some other error. Scrap it
      queue.removeByAddress(queueEntry);
      // make FD negative to tell parent thread we need to remove the client
      socketFD *= -1;
    }
    music.setPath(queueEntry->path.c_str());
    music.writeToPath();
    DEBUG_P(std::cout << "unlock queueEntry mutex\n");
  };

  process();

  // notify parent thread that this thread is done
  struct {
    int socketFD;
    MusicStorageEntry *p_queue;
  } t;
  t.socketFD = socketFD;
  t.p_queue = queueEntry;
  DEBUG_P(std::cout << "recv process done, writing to recv pipe: socketFD " << socketFD << " p_queue " << queueEntry << "\n");
  ::write(threadRecvPipe[1], reinterpret_cast<const void *>(&t), sizeof t);
}

void Room::sendBasicResponse(ThreadSafeSocket& socket, Command response) {
  Message message;
  message.setCommand(static_cast<std::byte>(response));
  socket.write(message.getMessage().data(), message.getMessage().size());
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
