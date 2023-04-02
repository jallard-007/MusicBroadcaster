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

using namespace clnt;

Client::Client():
  shouldRemoveFirstOnNext{false}, fdMax{}, threadPipe{}, clientName{},
  queue{}, audioPlayer{}, master{}, clientSocket{} {}

Client::Client(std::string name):
  shouldRemoveFirstOnNext{false}, fdMax{}, threadPipe{}, clientName{std::move(name)},
  queue{}, audioPlayer{}, master{}, clientSocket{} {}

Client::~Client() {
  if (threadPipe[0] != 0) {
    close(threadPipe[0]);
  }
  if (threadPipe[1] != 0) {
    close(threadPipe[1]);
  }
}

bool Client::initializeClient() {
  const uint16_t port = getPort();
  std::string host;
  getHost(host);
  if (!clientSocket.connect(host, port)) {
    return false;
  }

  if (::pipe(threadPipe) == -1) {
    fprintf(stderr, "pipe: %s (%d)\n", strerror(errno), errno);
    return false;
  }
  fdMax = clientSocket.getSocketFD();
  fdMax = fdMax > threadPipe[0] ? fdMax : threadPipe[0];

  FD_ZERO(&master);
  FD_SET(0, &master);
  FD_SET(clientSocket.getSocketFD(), &master);
  FD_SET(threadPipe[0], &master);

  std::cout << "Successfully joined the room\n";
  return true;
}

// TODO: if interrupted by server that requires a message to be printed, take what was written and put it back on command line after
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

    if (FD_ISSET(threadPipe[0], &read_fds)) { 
      processThreadFinished();
    }
  }
}

void Client::processThreadFinished() {
  DEBUG_P(std::cout << "data from thread pipe\n");
  PipeData_t t;
  ::read(threadPipe[0], reinterpret_cast<void *>(&t), sizeof t);
  if (t.fileDes < 0) {
    exit(1);
  }
  FD_SET(t.fileDes, &master);
}

enum class ClientCommand {
  FAQ,
  HELP,
  EXIT,
  QUIT,
  ADD_SONG,
  SEEK,
  MUTE,
  UNMUTE
};

const std::unordered_map<std::string, ClientCommand> clientCommandMap = {
  {"faq", ClientCommand::FAQ},
  {"help", ClientCommand::HELP},
  {"exit", ClientCommand::EXIT},
  {"quit", ClientCommand::QUIT},
  {"add song", ClientCommand::ADD_SONG},
  {"seek", ClientCommand::SEEK},
  {"mute", ClientCommand::MUTE},
  {"unmute", ClientCommand::UNMUTE},
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

    case ClientCommand::MUTE:
      audioPlayer.mute();
      break;

    case ClientCommand::UNMUTE:
      audioPlayer.unmute();
      break;

    default:
      // this section of code should never be reached
      std::cerr << "Error: Reached default case in Client::handleStdinCommand\nCommand " << input << " not handled but is in clientMapCommand\n";
      exit(1);
  }
  std::cout << " >> ";
  std::cout.flush();
  return true;
}

void Client::handleServerSongData_threaded(Message mes) {
  DEBUG_P(std::cout << "song data message from server of size" << mes.getBodySize() << "\n");
  auto musicEntry = queue.addAtIndexAndLock(static_cast<uint8_t>(mes.getOptions()));
  PipeData_t t = { clientSocket.getSocketFD() };
  auto process = [this, &musicEntry, &t](uint32_t bodySize) {
    Music music;
    music.getVector().resize(bodySize);
    if (!queue.makeTemp(musicEntry)) {
      std::cerr << "Error: makeTemp\n";
      t.fileDes *= -1;
    }
    if (clientSocket.readAll(music.getVector().data(), bodySize) == 0) {
      std::cerr << "lost connection to room\n";
      t.fileDes *= -1;
    }
    DEBUG_P(std::cout << "got song data\n");

    { // send received ok response to server
      Message response;
      response.setCommand((std::byte)Commands::Command::RECV_OK);
      clientSocket.write(response.data(), response.size());
    }
    DEBUG_P(std::cout << "sent back ok\n");
    // write the data to dis
    music.setPath(musicEntry->path);
    music.writeToPath();
    musicEntry->entryMutex.unlock();
  };


  process(mes.getBodySize());

  ::write(threadPipe[1], reinterpret_cast<const void *>(&t), sizeof t);
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
      FD_CLR(clientSocket.getSocketFD(), &master);
      std::thread thread = std::thread(&Client::handleServerSongData_threaded, this, mes);
      thread.detach();
      break;
    }
    
    case Commands::Command::PLAY_NEXT: {
      handleServerPlayNext();
      break;
    }

    case Commands::Command::RES_ADD_TO_QUEUE_OK: {
      DEBUG_P(std::cout << "got ok\n");
      FD_CLR(0, &master);
      std::thread thread = std::thread(
        &Client::sendMusicFile_threaded,
        this,
        static_cast<uint8_t>(mes.getOptions())
      );
      thread.detach();
      break;
    }

    case Commands::Command::RES_ADD_TO_QUEUE_NOT_OK:
      std::cout << "The room is not allowing you to upload, try again later\n";
      break;

    case Commands::Command::REMOVE_QUEUE_ENTRY: {
      DEBUG_P(std::cout << "remove by position " << (int)mes.getOptions() << '\n');
      queue.removeByPosition(static_cast<uint8_t>(mes.getOptions()));
      break;
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

void Client::sendMusicFile_threaded(uint8_t position) {
  DEBUG_P(std::cout << "sendMusicFile\n");
  PipeData_t t = { 0 };
  auto process = [this, &t, position]() {
    Music m;
    MusicStorageEntry *p_entry = queue.addAtIndexAndLock(position);
    getMP3FilePath(m);
    if (m.getPath() == "-1") {
      Message header;
      header.setCommand((std::byte)Commands::Command::CANCEL_REQ_ADD_TO_QUEUE);
      if (!clientSocket.writeHeaderAndData(header.data(), m.getVector().data(), m.getVector().size())) {
        DEBUG_P(std::cout << "couldn't send \n");
        t.fileDes = -1;
      }
      return;
    }

    m.readFileAtPath();
    p_entry->path = m.getPath();
    p_entry->entryMutex.unlock();
    DEBUG_P(std::cout << "unlocked entry mutex\n");

    Message header;
    header.setCommand(static_cast<std::byte>(Commands::Command::SONG_DATA));
    header.setBodySize(static_cast<uint32_t>(m.getVector().size()));
    DEBUG_P(std::cout << "sending data \n");
    if (!clientSocket.writeHeaderAndData(header.data(), m.getVector().data(), m.getVector().size())) {
      DEBUG_P(std::cout << "couldn't send \n");
      t.fileDes = -1;
      return;
    } 
    DEBUG_P(std::cout << "sent data \n");
    std::cout << "Added song to queue\n";
    p_entry->entryMutex.unlock();
    std::cout << " >> ";
    std::cout.flush();
  };

  process();

  ::write(threadPipe[1], reinterpret_cast<const void *>(&t), sizeof t);
}
