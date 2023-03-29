/**
 * @author Justin Nicolas Allard
 * Header file for client class
*/

#ifndef CLIENT_CLASS_H
#define CLIENT_CLASS_H

#include <string>

#include "../music/Player.hpp"
#include "../music/MusicStorage.hpp"
#include "../socket/ThreadSafeSocket.hpp"

class Client {
private:
  bool shouldRemoveFirstOnNext;
  int id;
  int fdMax;
  std::string clientName;
  ThreadSafeSocket clientSocket;
  MusicStorage queue;
  Player audioPlayer;
  fd_set master;

  /**
   * Asks for the path to the file.
   * Then sends the size of the file as well as the file itself through clientSocket
  */
  void sendMusicFile();

  bool handleStdinCommand();

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

  [[nodiscard]] const ThreadSafeSocket &getSocket() const;
  [[nodiscard]] const std::string &getName() const;
  void setName(std::string);
  int getId() const;
};

#endif
