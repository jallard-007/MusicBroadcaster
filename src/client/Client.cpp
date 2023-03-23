/**
 * \author Justin Nicolas Allard
 * Implementation file for client class
*/

#include <fstream>
#include <iostream>
#include "Client.hpp"
#include "../messaging/Commands.hpp"
#include "../music/Music.hpp"

Client::Client(const std::string &name):
  id{}, clientName{name}, clientSocket{} {}

Client::Client(const std::string &name, BaseSocket &&socket):
  id{}, clientName{name}, clientSocket{std::move(socket)} {}

const std::string &Client::getName() const  {
  return clientName;
}

void Client::setName(std::string name) {
  clientName = name;
}

int Client::getId() const {
  return id;
}

const BaseSocket &Client::getSocket() const {
  return clientSocket;
}

void Client::sendMusicFile() {
  if (clientSocket.getSocketFD() == 0) {
    std::cerr << "socket file descriptor not set\n";
    return;
  }
  
  Music music; // create music object
  std::string input;
  while (1) {
    std::cout << "Enter file path\n >> ";
    std::getline(std::cin, input);
    music.setPath(input);
    if (music.readFileAtPath()) {
      break; // file read successfully, exit loop
    }
  }

  // create request message
  Message message;
  message.setCommand((std::byte)Commands::Command::REQ_ADD_TO_QUEUE);
  const unsigned int sizeOfFile = (unsigned int)music.getBytes().size();
  message.setBodySize(sizeOfFile);
  auto formattedRequest = message.format();
  if (!clientSocket.write(formattedRequest.data(), formattedRequest.size())) {
    return; // writing to the socket failed
  }
  std::cout << "Sent file size to server\n";
  // await OK from room
  std::byte responseBuffer[6];
  if (!clientSocket.read(responseBuffer, sizeof responseBuffer)) {
    return; // reading from the socket failed
  }
  Message response(responseBuffer);
  if (static_cast<Commands::Command>(response.getCommand()) != Commands::Command::RES_OK) {
    std::cout << "Room is not allowing you to upload, try again later\n";
    return;
  }

  // do chunking here, make message for each chunk
  Message musicDataMessage;
  musicDataMessage.setCommand((std::byte)Commands::Command::SONG_DATA);
  musicDataMessage.setBodySize((unsigned int)music.getBytes().size());
  musicDataMessage.setBody(music.getBytes());
  auto f = musicDataMessage.format();
  if (!clientSocket.write(f.data(), f.size())) {
    return; // writing to the socket failed
  }
  std::cout << "Sent file contents to server\n";
}
