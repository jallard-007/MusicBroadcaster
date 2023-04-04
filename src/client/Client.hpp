/**
 * @file Client.hpp
 * @author Justin Nicolas Allard
 * @brief Header file for client class
 * @version 1.4
 * @date 2023-04-02
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#pragma once

#include <fstream>
#include <iostream>
#include <string>
#include <thread>
#include <cstdlib>
#include <unordered_map>
#if _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>

#elif defined(__APPLE__) || defined(__unix__)
#include <unistd.h>
#include <sys/select.h>
#endif

#include "../socket/BaseSocket.hpp"
#include "../socket/ThreadSafeSocket.hpp"
#include "../music/MusicStorage.hpp"
#include "../messaging/Message.hpp"
#include "../messaging/Commands.hpp"
#include "../music/Player.hpp"
#include "../music/Music.hpp"
#include "../CLInput.hpp"
#include "../debug.hpp"


// client
namespace clnt {

typedef struct {
  int fileDes;
} PipeData_t;

/**
 * @brief Client class. Handles the client
 * 
 */
class Client {
private:
  bool shouldRemoveFirstOnNext;
  int fdMax;
  int threadPipe[2];

  /**
   * @brief The name of the client
   * 
   */
  std::string clientName;
  MusicStorage queue;
  Player audioPlayer;
  fd_set master;

  /**
   * @brief The socket associated with the client
   * 
   */
  ThreadSafeSocket clientSocket;

  bool processThreadFinished();

  /**
  */
  void reqSendMusicFile();

  void sendMusicFile_threaded(uint8_t);

  int handleStdinCommand();

  void handleServerSongData_threaded(Message mes);

  bool handleServerPlayNext(Message &mes);

  bool handleServerMessage();

public:
  Client();
  ~Client();

  /**
   * Constructor
   * @param name name of client
  */
  explicit Client(std::string name);

  bool initializeClient(uint16_t, const std::string &);

  int handleClient();
};

}
