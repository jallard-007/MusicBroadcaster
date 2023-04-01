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

Room::Room(): ip{}, fdMax{}, hostSocket{},
  threadRecvPipe{}, threadSendPipe{}, threadWaitAudioPipe{},
  name{}, clients{}, queue{}, audioPlayer{}, master{} {}

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
  //audioPlayer.mute();
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
        if (!handleClientRequests(*client)) {
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

void Room::processThreadFinishedReceiving() {
  DEBUG_P(std::cout << "data from recv pipe\n");
  RecvPipeData_t t;
  ::read(threadRecvPipe[0], reinterpret_cast<void *>(&t), sizeof t);

  if (t.socketFD < 0) { // when true, means that we need to remove that client
    DEBUG_P(std::cout << "client disconnected\n");
    t.socketFD *= -1;
    // remove it
    clients.remove_if([&t](room::Client &client){
      return client.getSocket().getSocketFD() == t.socketFD;
    });
    return;
  }
  if (t.p_entry != nullptr) {
    sendSongToAllClients(t);
  } else {
    DEBUG_P(std::cout << "adding was cancelled, now adding client back to master\n");
    FD_SET(t.socketFD, &master);
  }
}

void Room::processThreadFinishedSending() {
  SendPipeData_t t;
  ::read(threadSendPipe[0], reinterpret_cast<void *>(&t), sizeof t);
  if (t.socketFD < 0) { // when true, means that we need to remove that client
    t.socketFD  *= -1;
    // remove it
    clients.remove_if([&t](room::Client &client){
      return &client == t.p_client;
    });
  } else { // other wise its all good, continue
    // add the client's socket FD back to select
    // check if any outstanding sending threads for this client
    --t.p_client->sendingCount;
    if (t.p_client->sendingCount == 0) {
      FD_SET(t.socketFD , &master);
    }
  }
}

void Room::sendSongDataToClient_threaded(
  std::shared_ptr<Music> audio,
  const MusicStorageEntry *p_queue,
  uint8_t queuePosition,
  room::Client *p_client
) {
  auto &clientSocket = p_client->getSocket();
  SendPipeData_t t {
    clientSocket.getSocketFD(),
    p_client,
    const_cast<MusicStorageEntry *>(p_queue)
  };

  // send file to client
  const auto &audioData = audio->getVector();
  Message message;
  message.setCommand(static_cast<std::byte>(Command::SONG_DATA));
  message.setOptions(static_cast<std::byte>(queuePosition));
  message.setBodySize(static_cast<uint32_t>(audioData.size()));
  DEBUG_P(std::cout << "sending file to client\n");
  if (!clientSocket.writeHeaderAndData(message.data(), audioData.data(), audioData.size())) {
    t.socketFD *= -1;
  } else {
    t.p_entry->sent++;
  }

  ::write(threadSendPipe[1], reinterpret_cast<const void *>(&t), sizeof t);
}

void Room::sendSongToAllClients(const RecvPipeData_t &next) {
  DEBUG_P(std::cout << "sending song to all clients\n");
  if (next.p_entry == nullptr) {
    DEBUG_P(std::cout << "song is nullptr\n");
    return;
  }
  if (next.p_entry->sent != 0) {
    DEBUG_P(std::cout << "song has already been sent, no need to send again\n");
    return;
  }
  if (!next.p_entry->entryMutex.try_lock()) {
    DEBUG_P(std::cout << "could not get mutex for song\n");
    return;
  }
  // 0 means we haven't started sending yet
  // 1 means that we have started sending, sent - 1 is the number of clients that it has been sent to
  next.p_entry->sent = 1;
  if (clients.empty() || (clients.size() == 1 && next.socketFD == clients.front().getSocket().getSocketFD())) {
    DEBUG_P(std::cout << "no one to send to\n");
    next.p_entry->entryMutex.unlock();
    attemptPlayNext();
  } else {
    Music m;
    m.setPath(next.p_entry->path);
    auto data = m.getMemShared();
    next.p_entry->entryMutex.unlock();
    int position = queue.getPositionInQueue(next.p_entry);
    if (position == -1) {
      std::cerr << "Error: entry not found\n";
      return;
    }
    for (room::Client &client : clients) {
      FD_CLR(client.getSocket().getSocketFD(), &master);
      // no need to send it back to the client that sent it
      if (client.getSocket().getSocketFD() != next.socketFD) {
        ++client.sendingCount;
        std::thread thread = std::thread(
          &Room::sendSongDataToClient_threaded,
          this,
          data,
          next.p_entry,
          static_cast<uint8_t>(position),
          &client
        );
        thread.detach();
      }
    }
  }
  // add client back to master, this could also be stdin
  FD_SET(next.socketFD, &master);
}

void Room::waitOnAudio_threaded() {
  DEBUG_P(std::cout << "waiting for audio to finish\n");
  audioPlayer.wait();
  DEBUG_P(std::cout << "audio finished\n");
  int nothing = 0;
  write(threadWaitAudioPipe[1], &nothing, sizeof nothing);
}

void Room::attemptPlayNext() {
  DEBUG_P(std::cout << "attempt play next\n");
  if (audioPlayer.isPlaying()) {
    DEBUG_P(std::cout << "audio still playing, cancel\n");
    return;
  }
  auto musicEntry = queue.getFront();
  if (musicEntry != nullptr){
    if (!musicEntry->entryMutex.try_lock()) {
      DEBUG_P(std::cout << "could not get queue entry mutex, cancelling\n");
      return;
    }
    DEBUG_P(std::cout << "got queue entry mutex\n");
  }

  DEBUG_P(std::cout << "sending play next message to all clients\n");
  for (room::Client &client : clients) {
    Message message;
    message.setCommand(Command::PLAY_NEXT);
    client.getSocket().write(message.data(), message.size());
  }
  if (musicEntry != nullptr) {
    DEBUG_P(std::cout << "feeding next in queue to audioPlayer\n");
    audioPlayer.feed(musicEntry->path.c_str());
    audioPlayer.play();
    std::thread threadAudioWait = std::thread(&Room::waitOnAudio_threaded, this);
    threadAudioWait.detach();
    musicEntry->entryMutex.unlock();
    DEBUG_P(std::cout << "unlocked queue entry mutex\n");
  }
}

void Room::handleClientReqSongData_threaded(room::Client *p_client, uint32_t sizeOfFile) {
  ThreadSafeSocket &socket = p_client->getSocket();
  RecvPipeData_t t {
    socket.getSocketFD(),
    p_client->p_entry
  };

  auto process = [this, &socket, &t, sizeOfFile]() {
    DEBUG_P(std::cout << "res ok, now getting the song\n");
    // read in the header, create message object from it
    // get size of message from message object
    DEBUG_P(std::cout << "got header, now reading in file of size " << sizeOfFile << "\n");
    Music music;
    music.getVector().resize(sizeOfFile);

    std::byte *dataPointer = music.getVector().data();
    const auto numBytesRead = static_cast<uint32_t>(socket.readAll(dataPointer, sizeOfFile));
    if (numBytesRead <= 0) {
      // either client disconnected half way through, or some other error. Scrap it
      DEBUG_P(std::cout << "error reading song from socket, removing entry from queue\n");
      queue.removeByAddress(t.p_entry);
      t.p_entry = nullptr;
      // make FD negative to tell parent thread we need to remove the client
      t.socketFD *= -1;
      return;
    }
    music.setPath(t.p_entry->path);
    music.writeToPath();
    t.p_entry->entryMutex.unlock();
    DEBUG_P(std::cout << "unlocked queueEntry mutex\n");
  };

  process();

  // notify parent thread that this thread is done
  DEBUG_P(std::cout << "recv process done, writing to recv pipe: socketFD " << t.socketFD << "\n");
  ::write(threadRecvPipe[1], reinterpret_cast<const void *>(&t), sizeof t);
}

void Room::handleClientReqAddQueue(room::Client &client) {
  DEBUG_P(std::cout << "req add to queue request\n");

  auto p_entry = this->queue.addAndLockEntry();

  if (p_entry == nullptr) {
    // adding to queue was unsuccessful
    // send a message back to client to deny their request to add a song
    sendBasicResponse(client.getSocket(), Command::RES_ADD_TO_QUEUE_NOT_OK);
    DEBUG_P(std::cout << "res not ok, no room in queue\n");
    return;
  }

  // adding to the queue was successful
  // send a message back to client to confirm that they can continue to send the song
  int position = queue.getPositionInQueue(p_entry);
  if (position == -1) {
    std::cerr << "Error: couldn't find the entry\n";
    exit(1);
  }
  client.p_entry = p_entry;
  sendBasicResponse(client.getSocket(), Command::RES_ADD_TO_QUEUE_OK, static_cast<std::byte>(position));
  DEBUG_P(std::cout << "res ok\n");
}

void Room::handleCancelReqAddQueue(MusicStorageEntry *p_entry) {
  if (p_entry == nullptr) {
    return;
  }
  const int position = queue.getPositionInQueue(p_entry);
  if (position < 0) {
    return;
  }
  queue.removeByAddress(p_entry);
  Message message;
  message.setCommand(Command::REMOVE_QUEUE_ENTRY);
  message.setOptions(static_cast<std::byte>(position));
  for (room::Client &client : clients) {
    client.getSocket().write(message.data(), message.size());
  }
}

bool Room::handleClientRequests(room::Client &client) {
  std::byte requestHeader[SIZE_OF_HEADER];
  const size_t numBytesRead = client.getSocket().readAll(requestHeader, SIZE_OF_HEADER);
  if (numBytesRead == 0) {
    return false;
  }
  DEBUG_P(std::cout << "read client request\n");
  // convert header to message
  Message message(requestHeader);
  // handle every supported message here
  Command command = message.getCommand();
  switch(command) {
    case Command::REQ_ADD_TO_QUEUE:
      handleClientReqAddQueue(client);
      break;

    case Command::RECV_OK:
      DEBUG_P(std::cout << "got recv ok from client\n");
      attemptPlayNext();
      break;

    case Command::CANCEL_REQ_ADD_TO_QUEUE:
      handleCancelReqAddQueue(client.p_entry);
      client.p_entry = nullptr;
      break;

    case Command::SONG_DATA: {
      if (client.p_entry == nullptr) {
        // client does not have a spot in the queue
        break;
      }
      FD_CLR(client.getSocket().getSocketFD(), &master);
      std::thread clientThread = std::thread(
        &Room::handleClientReqSongData_threaded,
        this,
        &client,
        message.getBodySize()
      );
      clientThread.detach();
    }

    default:
      DEBUG_P(std::cout << "bad request\n");
      sendBasicResponse(client.getSocket(), Command::BAD_VALUES);
      break;
  }
  return true;
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

  auto &client = addClient({"user", std::move(clientSocket)});

  auto &songs = queue.getSongs();
  int position = -1;
  Music m;
  for (const MusicStorageEntry &entry : songs) {
    ++position;
    m.setPath(entry.path);
    
    if (&entry == queue.getFront()) {
      // decide if its worth it to send the first song
    } else if (entry.sent > 0) {
      auto data = m.getMemShared();
      if (data == nullptr) {
        continue;
      }
      ++client.sendingCount;
      std::thread thread = std::thread(
        &Room::sendSongDataToClient_threaded,
        this,
        data,
        &entry,
        static_cast<uint8_t>(position),
        &client
      );
      thread.detach();
    }
  }

  if (client.sendingCount == 0) {
    FD_SET(client.getSocket().getSocketFD(), &master);
  }
  // add the client to the selector list
}

void Room::handleStdinAddSongHelper_threaded(MusicStorageEntry *queueEntry) {
  RecvPipeData_t t{0, queueEntry};
  auto process = [&t, this]() {
    Music m;
    getMP3FilePath(m);
    if (m.getPath() == "-1") {
      handleCancelReqAddQueue(t.p_entry);
      t.p_entry = nullptr;
      return;
    }
    t.p_entry->path = m.getPath();
    t.p_entry->entryMutex.unlock();
    DEBUG_P(std::cout << "unlocked entry mutex\n");
    std::cout << "Added song to queue\n";
  };

  if (t.p_entry != nullptr) {
    process();
  }

  DEBUG_P(std::cout << "add local song to queue process done, writing to recv pipe: socketFD " << t.socketFD << "\n");
  ::write(threadRecvPipe[1], reinterpret_cast<const void *>(&t), sizeof t);

  std::cout << " >> ";
  std::cout.flush();
}

void Room::handleStdinAddSong() {
  // clear stdin from master
  MusicStorageEntry *queueEntry = queue.addLocalAndLockEntry();
  if (queueEntry == nullptr) {
    std::cerr << "Unable to add a song to the queue\n";
    return;
  }

  FD_CLR(0, &master);
  std::thread addSongThread = std::thread(&Room::handleStdinAddSongHelper_threaded, this, queueEntry);
  addSongThread.detach();
}

enum class RoomCommand {
  FAQ,
  HELP,
  EXIT,
  QUIT,
  ADD_SONG,
  SEEK,
};

const std::unordered_map<std::string, RoomCommand> roomCommandMap = {
  {"faq", RoomCommand::FAQ},
  {"help", RoomCommand::HELP},
  {"exit", RoomCommand::EXIT},
  {"quit", RoomCommand::QUIT},
  {"add song", RoomCommand::ADD_SONG},
  {"seek", RoomCommand::SEEK},
};

// TODO:
void roomShowHelp() {
  std::cout <<
  "List of commands as room host:\n\n";
}

// TODO:
void roomShowFAQ() {
  std::cout <<
  "Question 1:\n\n";
}

bool Room::handleStdinCommands() {
  std::string input;
  std::getline(std::cin, input);
  RoomCommand command;
  try {
    command = roomCommandMap.at(input);
  } catch (const std::out_of_range &err){
    std::cout << "Invalid command. Try 'help' for information\n >> ";
    std::cout.flush();
    return true;
  }

  switch (command) {
    case RoomCommand::FAQ:
      roomShowFAQ();
      break;

    case RoomCommand::HELP:
      roomShowHelp();
      break;

    case RoomCommand::EXIT:
      return false;

    case RoomCommand::QUIT:
      queue.~MusicStorage();
      exit(0);

    case RoomCommand::ADD_SONG: {
      handleStdinAddSong();
      return true;
    }

    case RoomCommand::SEEK:
      break;

    default:
      // this section of code should never be reached
      std::cerr << "Error: Reached default case in Room::handleStdinCommands\nCommand " << input << " not handled but is in clientMapCommand\n";
      exit(1);
  }
  std::cout << " >> ";
  std::cout.flush();
  return true;
}

void Room::sendBasicResponse(ThreadSafeSocket& socket, Command response, std::byte option) {
  Message message;
  message.setCommand(static_cast<std::byte>(response));
  message.setOptions(option);
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
