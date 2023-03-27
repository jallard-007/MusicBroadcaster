/**
 * @author Justin Nicolas Allard
 * Implementation file for client class
*/

#include <fstream>
#include <iostream>
#include <cstring>
#include <cstddef>
#include <utility>
#include <sys/select.h>

#include "Client.hpp"
#include "../CLInput.hpp"
#include "../messaging/Commands.hpp"

Client::Client():
  id{}, fdMax{}, clientName{},
  clientSocket{}, queue{}, audioPlayer{}, master{} {}

Client::Client(std::string name):
  id{}, fdMax{}, clientName{std::move(name)},
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
  return true;
}

void Client::handleClient() {
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

bool Client::handleStdinCommand() {
  // handle command line input here
  std::string input;
  std::getline(std::cin, input);
  if (input == "exit") {
    audioPlayer.pause();
    return false;
  } else if (input == "send song") {
    sendMusicFile();
  } else if (input == "play") {
    FILE *fp = queue.getFront();
    if (fp != nullptr) {
      audioPlayer.feed(fp);
      audioPlayer.play();
      queue.removeFront();
    }
  }
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
  auto command = static_cast<Commands::Command>(mes.getCommand());
  switch (command) {
    case Commands::Command::SONG_DATA: {
      FILE *fp = queue.add();
      Music music;
      music.getBytes().resize(size);
      if (clientSocket.readAll(music.getBytes().data(), size) == 0) {
        audioPlayer.pause();
        std::cout << "lost connection to room\n";
        return false;
      }
      music.writeToFile(fp);
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
    Message message;
    message.setCommand((std::byte)Commands::Command::REQ_ADD_TO_QUEUE);
    auto formattedRequest = message.format();
    if (!clientSocket.write(formattedRequest.data(), formattedRequest.size())) {
      return; // writing to the socket failed
    }
    std::cout << "Sent queue song request to server\n";
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
    std::cout << "Enter file path\n >> ";
    std::getline(std::cin, input);
    music.setPath(input);
    if (music.readFileAtPath()) {
      break; // file read successfully, exit loop
    }
  }
  Message header;
  header.setCommand((std::byte)Commands::Command::SONG_DATA);
  header.setBodySize((uint32_t)music.getBytes().size());
  if (!clientSocket.writeHeaderAndData(header.format().data(), music.getBytes().data(), music.getBytes().size())) {
    return; // writing to the socket failed
  }
  std::cout << "Sent file contents to server\n";
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


