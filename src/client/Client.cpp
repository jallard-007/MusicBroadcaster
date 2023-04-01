/**
 * @author Justin Nicolas Allard
 * Implementation file for client class
*/

#include <fstream>
#include <iostream>
#include <cstring>
#include <cstddef>
#include <utility>
#include <unistd.h>
#include <sys/select.h>
#include <unordered_map>

#include "../debug.hpp"
#include "Client.hpp"
#include "../CLInput.hpp"
#include "../messaging/Message.hpp"

Client::Client():
  shouldRemoveFirstOnNext{false}, clientName{},
  queue{}, audioPlayer{}, master{}, clientSocket{} {}

Client::Client(std::string name):
  shouldRemoveFirstOnNext{false}, clientName{std::move(name)},
  queue{}, audioPlayer{}, master{}, clientSocket{} {}

bool Client::initializeClient() {
  const uint16_t port = getPort();
  std::string host;
  getHost(host);
  if (!clientSocket.connect(host, port)) {
    return false;
  }

  FD_ZERO(&master);
  FD_SET(0, &master);
  FD_SET(clientSocket.getSocketFD(), &master);
  std::cout << "Successfully joined the room\n";
  return true;
}

void Client::handleClient() {
  std::cout << " >> ";
  std::cout.flush();
  while (true) {
    fd_set read_fds = master;  // temp file descriptor list for select()
    if (::select(clientSocket.getSocketFD() + 1, &read_fds, nullptr, nullptr, nullptr /* <- time out in microseconds*/) == -1){
      fprintf(stderr, "select: %s (%d)\n", strerror(errno), errno);
      return;
    }

    if (FD_ISSET(clientSocket.getSocketFD(), &read_fds)) {
      DEBUG_P(std::cout << "server message\n");
      if (!handleServerMessage()) {
        return;
      }
    }

    // input from stdin, local user entered a command
    if (FD_ISSET(0, &read_fds)) { 
      if (!handleStdinCommand()) {
        return;
      }
    }
  }
}

enum class ClientCommand {
  FAQ,
  HELP,
  EXIT,
  QUIT,
  ADD_SONG,
  SEEK,
};

const std::unordered_map<std::string, ClientCommand> clientCommandMap = {
  {"faq", ClientCommand::FAQ},
  {"help", ClientCommand::HELP},
  {"exit", ClientCommand::EXIT},
  {"quit", ClientCommand::QUIT},
  {"add song", ClientCommand::ADD_SONG},
  {"seek", ClientCommand::SEEK},
};

// TODO:
void clientShowHelp() {
  std::cout <<
  "List of commands as client:\n\n";
}

// TODO:
void clientShowFAQ() {
  std::cout <<
  "Question 1:\n\n";
}

bool Client::handleStdinCommand() {
  std::string input;
  std::getline(std::cin, input);
  ClientCommand command;
  try {
    command = clientCommandMap.at(input);
  } catch (const std::out_of_range &err){
    std::cout << "Invalid command. Try 'help' for information\n >> ";
    std::cout.flush();
    return true;
  }

  switch (command) {
    case ClientCommand::FAQ:
      clientShowFAQ();
      break;

    case ClientCommand::HELP:
      clientShowHelp();
      break;

    case ClientCommand::EXIT:
      return false;

    case ClientCommand::QUIT:
      queue.~MusicStorage();
      exit(0);

    case ClientCommand::ADD_SONG:
      DEBUG_P(std::cout << "add song command\n");
      reqSendMusicFile();
      return true;

    case ClientCommand::SEEK: {
      // std::cout << "Enter a time to seek to:\n >> ";
      // std::string input;
      // std::getline(std::cin, input);
      // char *endPtr;
      // const auto time = strtof(input.c_str(), &endPtr);
      // if (*endPtr == '\0') {
      //   audioPlayer.seek(time);
      // }
      break;
    }

    default:
      // this section of code should never be reached
      std::cerr << "Error: Reached default case in Client::handleStdinCommand\nCommand " << input << " not handled but is in clientMapCommand\n";
      exit(1);
  }
  std::cout << " >> ";
  std::cout.flush();
  return true;
}

bool Client::handleServerSongData(Message &mes) {
  DEBUG_P(std::cout << "song data message from server of size" << mes.getBodySize() << "\n");
  Music music;
  music.getVector().resize(mes.getBodySize());
  // will want to eventually thread this off
  if (clientSocket.readAll(music.getVector().data(), mes.getBodySize()) == 0) {
    std::cerr << "lost connection to room\n";
    return false;
  }
  DEBUG_P(std::cout << "got song data\n");

  { // send received ok response to server
    Message response;
    response.setCommand((std::byte)Commands::Command::RECV_OK);
    clientSocket.write(response.data(), response.size());
  }
  DEBUG_P(std::cout << "sent back ok\n");

  auto musicEntry = queue.getFirstEmptyAndLockEntry();
  if (musicEntry == nullptr) {
    DEBUG_P(std::cout << "de-synced with server");
    exit(1);
  }

  // write the data to disk
  music.setPath(musicEntry->path);
  music.writeToPath();
  musicEntry->entryMutex.unlock();
  return true;
}

void Client::handleServerPlayNext() {
  DEBUG_P(std::cout << "play next message from server\n");
  if (audioPlayer.isPlaying()) {
    audioPlayer.pause();
  }
  if (shouldRemoveFirstOnNext) {
    DEBUG_P(std::cout << "remove front first\n");
    queue.removeFront();
  }
  auto nextSongFileName = queue.getFront();

  if (nextSongFileName != nullptr) {
    DEBUG_P(std::cout << "feeding next\n");
    audioPlayer.feed(nextSongFileName->path.c_str());
    audioPlayer.play();
    shouldRemoveFirstOnNext = true;
  } else {
    DEBUG_P(std::cout << "nothing to feed\n");
    shouldRemoveFirstOnNext = false;
  }
}

bool Client::handleServerMessage() {
  std::byte responseHeader[SIZE_OF_HEADER];
  if (clientSocket.readAll(responseHeader, SIZE_OF_HEADER) == 0){
    audioPlayer.pause();
    std::cout << "lost connection to room\n";
    return false;
  }
  Message mes(responseHeader);
  const auto command = static_cast<Commands::Command>(mes.getCommand());
  switch (command) {
    // always take the next queue entry, if there are none available, add one
    case Commands::Command::SONG_DATA: {
      return handleServerSongData(mes);
    }
    
    case Commands::Command::PLAY_NEXT: {
      handleServerPlayNext();
      break;
    }

    case Commands::Command::ADD_EMPTY_TO_QUEUE: {
      queue.addEmpty();
      break;
    }

    case Commands::Command::RES_ADD_TO_QUEUE: {
      std::byte option = mes.getOptions();
      if (option == RES_ADD_TO_QUEUE_OK) {
        DEBUG_P(std::cout << "got ok\n");
        sendMusicFile();
      } else{
        std::cout << "The room is not allowing you to upload, try again later\n";
      }
    }

    default:
      break;
  }
  return true;
}

void Client::reqSendMusicFile() {
  if (clientSocket.getSocketFD() == 0) {
    std::cerr << "socket file descriptor not set\n";
    return;
  }

  DEBUG_P(std::cout << "sending req add to queue to server\n");
  Message request;
  request.setCommand((std::byte)Commands::Command::REQ_ADD_TO_QUEUE);
  if (!clientSocket.write(request.data(), request.size())) {
    return; // writing to the socket failed
  }
}

void Client::sendMusicFile() {
  Music m;
  MusicStorageEntry *p_entry = queue.addLocalAndLockEntry();
  auto process = [this, &p_entry, &m]() {
    getMP3FilePath(m);
    if (m.getPath() == "-1") {
      queue.removeByAddress(p_entry);
      p_entry = nullptr;
      // TODO: need to add cancel command, so that client thread on server side does not freeze
      return;
    }
    m.readFileAtPath();
    p_entry->path = m.getPath();
    p_entry->entryMutex.unlock();
    DEBUG_P(std::cout << "unlocked entry mutex\n");
  };

  process();

  Message header;
  header.setCommand((std::byte)Commands::Command::SONG_DATA);
  header.setBodySize((uint32_t)m.getVector().size());
  DEBUG_P(std::cout << "sending data \n");
  if (!clientSocket.writeHeaderAndData(header.data(), m.getVector().data(), m.getVector().size())) {
    DEBUG_P(std::cout << "couldn't send \n");
    std::cerr << "There was an issue communicating the with server\n";
    exit(1);
  } else {
    DEBUG_P(std::cout << "sent data \n");
  }
  std::cout << "Added song to queue\n";
  p_entry->entryMutex.unlock();
  std::cout << " >> ";
  std::cout.flush();
}
