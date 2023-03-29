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

#include "Client.hpp"
#include "../CLInput.hpp"
#include "../messaging/Message.hpp"

Client::Client():
  shouldRemoveFirstOnNext{false}, id{}, fdMax{}, clientName{},
  clientSocket{}, queue{}, audioPlayer{}, master{} {}

Client::Client(std::string name):
  shouldRemoveFirstOnNext{false}, id{}, fdMax{}, clientName{std::move(name)},
  clientSocket{}, queue{}, audioPlayer{}, master{} {}

bool Client::initializeClient() {
  const uint16_t port = getPort();
  std::string host;
  getHost(host);
  if (!clientSocket.connect(host, port)) {
    return false;
  }

  fdMax = clientSocket.getSocketFD();

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
    if (::select(fdMax + 1, &read_fds, nullptr, nullptr, nullptr /* <- time out in microseconds*/) == -1){
      fprintf(stderr, "select: %s (%d)\n", strerror(errno), errno);
      return;
    }

    if (FD_ISSET(clientSocket.getSocketFD(), &read_fds)) {
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
  ADD_SONG,
  SEEK,
};

const std::unordered_map<std::string, ClientCommand> clientCommandMap = {
  {"faq", ClientCommand::FAQ},
  {"help", ClientCommand::HELP},
  {"exit", ClientCommand::EXIT},
  {"add song", ClientCommand::ADD_SONG},
  {"seek", ClientCommand::SEEK},
};

// TODO:
void clientShowHelp() {
  std::cout <<
  "List of commands as client:\n\n";
}

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
    std::cout << "Invalid command. Try 'help' for information\n";
    std::cout << " >> ";
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
      queue.~MusicStorage();
      exit(0);

    case ClientCommand::ADD_SONG:
      sendMusicFile();
      break;

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

bool Client::handleServerMessage() {
  std::byte responseHeader[6];
  if (clientSocket.readAll(responseHeader, 6) == 0){
    audioPlayer.pause();
    std::cout << "lost connection to room\n";
    return false;
  }
  Message mes(responseHeader);
  const uint32_t size = mes.getBodySize();
  const auto command = static_cast<Commands::Command>(mes.getCommand());
  switch (command) {
    case Commands::Command::SONG_DATA: {
      Music music;
      music.getVector().resize(size);
      // will want to eventually thread this off
      if (clientSocket.readAll(music.getVector().data(), size) == 0) {
        audioPlayer.pause();
        std::cout << "lost connection to room\n";
        return false;
      }

      { // send received ok response to server
        Message response;
        response.setCommand((std::byte)Commands::Command::RECV_OK);
        clientSocket.write(response.data(), response.size());
      }

      auto musicEntry = queue.add();
      // write the data to disk
      if (musicEntry != nullptr) {
        music.setPath(musicEntry->path.c_str());
        music.writeToPath();
      } else {
        // we got a problem
      }
      break;
    }
    
    case Commands::Command::PLAY_NEXT: {
      if (audioPlayer.isPlaying()) {
        audioPlayer.pause();
      }
      if (shouldRemoveFirstOnNext) {
        queue.removeFront();
      }
      // TODO: need to check if there was a previous song that finished then remove it from the queue
      auto nextSongFileName = queue.getFront();

      if (nextSongFileName != nullptr) {
        audioPlayer.feed(nextSongFileName->path.c_str());
        Music m;
        m.setPath(nextSongFileName->path);
        m.readFileAtPath();
        std::cout << "path: " << nextSongFileName->path << '\n';
        std::cout << "size: " << m.getVector().size() << '\n';
        audioPlayer.play();
        shouldRemoveFirstOnNext = true;
      } else {
        shouldRemoveFirstOnNext = false;
      }
      break;
    }

    default:
      break;
  }
  return true;
}

void Client::sendMusicFile() {
  if (clientSocket.getSocketFD() == 0) {
    std::cerr << "socket file descriptor not set\n";
    return;
  }

  {
    // create request message
    Message request;
    request.setCommand((std::byte)Commands::Command::REQ_ADD_TO_QUEUE);
    if (!clientSocket.write(request.data(), request.size())) {
      return; // writing to the socket failed
    }
  }

  {
    // await OK from room
    std::byte responseBuffer[6];
    if (clientSocket.readAll(responseBuffer, sizeof responseBuffer) == 0) {
      // disconnected
      return;
    }
    Message response(responseBuffer);
    if (static_cast<Commands::Command>(response.getCommand()) != Commands::Command::RES_OK) {
      std::cout << "Room is not allowing you to upload, try again later\n";
      return;
    }
  }
  
  Music music; // create music object
  std::string input;
  while (true) {
    std::cout << "Enter file path:\n >> ";
    std::getline(std::cin, input);
    music.setPath(input);
    if (music.readFileAtPath()) {
      break; // file read successfully, exit loop
    }
  }
  Message header;
  header.setCommand((std::byte)Commands::Command::SONG_DATA);
  header.setBodySize((uint32_t)music.getVector().size());
  if (!clientSocket.writeHeaderAndData(header.data(), music.getVector().data(), music.getVector().size())) {
    return; // writing to the socket failed
  }
}

const std::string &Client::getName() const  {
  return clientName;
}

void Client::setName(std::string name) {
  clientName = std::move(name);
}

int Client::getId() const {
  return id;
}

const ThreadSafeSocket &Client::getSocket() const {
  return clientSocket;
}


