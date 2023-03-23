/**
 * \author Justin Nicolas Allard
 * Header file for room class
*/

#ifndef ROOM_CLASS_H
#define ROOM_CLASS_H

#include <string>
#include <list>
#include <mutex>
#include <thread>
#include "Client.hpp"
#include "../socket/BaseSocket.hpp"
#include "../music/MusicStorage.hpp"
#include "../messaging/Commands.hpp"

class Room {
private:

  /**
   * ip address of the room. this is not being used at the moment
  */
  int ip;

  /**
   * keep track of the biggest file descriptor for select
  */
  int fdMax;

  /**
   * Socket in which connections are established
  */
  BaseSocket hostSocket;

  /**
   * name of the room, also not being used
  */
  std::string name;

  /**
   * list of clients which have connected to the room
  */
  std::list<room::Client> clients;

  /**
   * queue of music
  */
  MusicStorage queue;

  /**
   * master file descriptor list
  */
  fd_set master;

  /**
   * Handles connection requests
  */
  void _handleConnectionRequests();

  /**
   * Handles incoming messages from the client.
   * \param client reference to client object
  */
  void _handleClientRequest(room::Client& client, const std::byte *requestHeader);

  /**
   * Attempts to add a song to the queue
   * \param client the client object to communicate with
   * \param sizeOfFile the size of the audio file
  */
  void _attemptAddSongToQueue(room::Client& client, size_t sizeOfFile);

  /**
   * Sends a header only response to the client
   * \param socket socket to send to
   * \param responseCommand one of the responses define in Commands.hpp
  */
  void _sendBasicResponse(BaseSocket& socket, Commands::Command responseCommand);

public:

  /**
   * Default Constructor
  */
  Room();

  ~Room() = default;

  /**
   * manages the room
  */
  void manageRoom();

  /**
   * set ip
   * \param newIp new ip number
  */
  void setIp(int newIp);

  /**
   * print all clients
  */
  void printClients();

  /**
   * add a client
   * \param client client object to add
   * \returns reference to client which was added
  */
  room::Client &addClient(room::Client &&client);


  /**
   * Various getters
  */

  [[nodiscard]] MusicStorage &getQueue();
  [[nodiscard]] const room::Client *searchClient(int);
  [[nodiscard]] int getIp() const;
  [[nodiscard]] const std::string &getName() const;
  [[nodiscard]] const std::list<room::Client> &getClients() const;
};

#endif
