/**
 * @author Justin Nicolas Allard
 * Header file for room class
*/

#ifndef ROOM_CLASS_H
#define ROOM_CLASS_H

#include <string>
#include <list>
#include <mutex>
#include <sys/select.h>
#include "Client.hpp"
#include "../socket/BaseSocket.hpp"
#include "../music/MusicStorage.hpp"
#include "../music/Player.hpp"
#include "../messaging/Commands.hpp"
#include "../socket/ThreadSafeSocket.hpp"

typedef struct {
    int socketFD;
    MusicStorageEntry *p_queue;
} RecvPipe_t;

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
   * number of clients that have fully received the current song
  */
  uint32_t numClientsReceivedFile;

  /**
   * Socket in which connections are established
  */
  BaseSocket hostSocket;

  /**
   * Pipe to communicate from child threads to parent thread.
   * This is used to allow child threads to notify when they have completed with select
  */
  int threadRecvPipe[2];

  /**
   * Pipe to communicate from child threads to parent thread.
   * This is used to allow child threads to notify when they have completed with select
  */
  int threadSendPipe[2];

  /**
   * 
  */
  int threadWaitAudioPipe[2];

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
   * plays audio
  */
  Player audioPlayer;

  /**
   * master file descriptor list
  */
  fd_set master;

  /**
   * Handles connection requests
  */
  void handleConnectionRequests();

  /**
   * Handles incoming messages from the client.
   * @param client reference to client object
   * @returns false if the client should be removed, true otherwise
  */
  bool handleClientRequest(room::Client& client);

  void handleStdinAddSong();

  /**
   * Handles all stdin input
  */
  bool handleStdinCommands();

  /**
   * 
  */
  void processThreadFinishedReceiving();

  /**
   * 
  */
  void processThreadFinishedSending();

  /**
   * Sends song data to a specific client
  */
  void sendSongDataToClient(std::shared_ptr<Music> audio, room::Client &client);

  /**
   * Attempts to send the next song to all clients client
  */
  void sendSongToAllClients(const RecvPipe_t &);

  /**
   * Attempts to add a song to the queue
   * @param clientPtr pointer to the client object to communicate with
  */
  void handleREQ_ADD_TO_QUEUE(room::Client *clientPtr);

  void waitOnAudio();

  void attemptPlayNext();

  /**
   * Sends a header only response to the client
   * @param socket socket to send to
   * @param responseCommand one of the responses define in Commands.hpp
  */
  static void sendBasicResponse(ThreadSafeSocket& socket, Commands::Command responseCommand, std::byte option = (std::byte)0);

public:

  /**
   * Default Constructor
  */
  Room();

  ~Room();

  /**
   * Initializes the room, preparing it to launch
   * @returns false on error, true on success
  */
  bool initializeRoom();

  /**
   * launch the room. Accepts incoming connections and manages the room
  */
  void launchRoom();

  /**
   * set ip
   * @param newIp new ip number
  */
  void setIp(int newIp);

  /**
   * print all clients
  */
  void printClients();

  /**
   * add a client
   * @param client client object to add
   * @returns reference to client which was added
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
