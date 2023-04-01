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

class Client {
private:
  bool shouldRemoveFirstOnNext;
  std::string clientName;
  MusicStorage queue;
  Player audioPlayer;
  fd_set master;
  ThreadSafeSocket clientSocket;

  /**
  */
  void reqSendMusicFile();

  void sendMusicFile();

  bool handleStdinCommand();

  bool handleServerSongData(Message &mes);

  void handleServerPlayNext();

  bool handleServerMessage();

public:
  Client();
  ~Client() = default;

  /**
   * Constructor
   * @param name name of client
  */
  explicit Client(std::string name);

  bool initializeClient();

  void handleClient();
};

#endif
