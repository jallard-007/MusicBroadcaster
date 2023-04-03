/**
 * @author Justin Nicolas Allard
 * Header file for client class
*/

#ifndef CLIENT_CLASS_H
#define CLIENT_CLASS_H

#include <string>

#include "../music/Player.hpp"
#include "../messaging/Message.hpp"
#include "../music/MusicStorage.hpp"
#include "../socket/ThreadSafeSocket.hpp"

// client
namespace clnt {

typedef struct {
  int fileDes;
} PipeData_t;

class Client {
private:
  bool shouldRemoveFirstOnNext;
  int fdMax;
  int threadPipe[2];
  std::string clientName;
  MusicStorage queue;
  Player audioPlayer;
  fd_set master;
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

  bool initializeClient();

  int handleClient();
};

}

#endif
