/**
 * @author Justin Nicolas Allard
 * Header file for room class
 */

#pragma once

#include <string>
#include <list>
#include <mutex>
#include <iostream>
#include <vector>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <thread>
#include <unordered_map>

#if _WIN32
// windows includes
#elif defined(__APPLE__) || defined(__unix__)
#include <sys/select.h>
#include <unistd.h>
#endif

#include "Client.hpp"
#include "../socket/BaseSocket.hpp"
#include "../music/MusicStorage.hpp"
#include "../music/Player.hpp"
#include "../CLInput.hpp"
#include "../debug.hpp"
#include "../messaging/Message.hpp"
#include "../messaging/Commands.hpp"
#include "../socket/ThreadSafeSocket.hpp"
#include "../tracker/TrackerAPI.hpp"

namespace room {

typedef struct {
  int socketFD;
  room::Client *p_client;
  MusicStorageEntry *p_entry;
} SendPipeData_t;

typedef struct {
  int socketFD;
  MusicStorageEntry *p_entry;
} RecvPipeData_t;

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
   * 
  */
  int64_t startTime;

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
   * @brief Handles connection requests
  */
  void handleConnectionRequests();

  void handleCancelReqAddQueue(MusicStorageEntry *);

  /**
  */
  void handleClientReqSongData_threaded(room::Client *p_client, uint32_t sizeOfFile);

  void handleClientReqAddQueue(room::Client &client);


  /**
   * @brief Handles incoming messages from the client.
   * @param client reference to client object
   * @returns false if the client should be removed, true otherwise
  */
  bool handleClientRequests(room::Client& client);

  void handleStdinAddSongHelper_threaded(MusicStorageEntry *queueEntry);

  void handleStdinAddSong();

  /**
   * @brief Handles all stdin input
  */
  int handleStdinCommands();

  /**
   * 
  */
  void processThreadFinishedReceiving();

  /**
   * 
  */
  void processThreadFinishedSending();

  /**
   * @brief Sends song data to a specific client
  */
  void sendSongDataToClient_threaded(
    std::shared_ptr<Music> audio,
    const MusicStorageEntry *p_queue,
    uint8_t queuePosition,
    room::Client *p_client
  );

  /**
   * @brief Attempts to send the next song to all clients client
  */
  void sendSongToAllClients(const RecvPipeData_t &);

  void waitOnAudio_threaded();

  void attemptPlayNext();

  /**
   * @brief Sends a header only response to the client
   * @param socket socket to send to
   * @param responseCommand one of the responses define in Commands.hpp
  */
  static void sendBasicResponse(ThreadSafeSocket& socket, Commands::Command responseCommand, std::byte option = (std::byte)0);

public:

  /**
   * @brief Default Constructor
  */
  Room();

  ~Room();

  /**
   * @brief Initializes the room, preparing it to launch
   * @returns false on error, true on success
  */
  bool initializeRoom();

  /**
   * @brief launch the room. Accepts incoming connections and manages the room
  */
  bool launchRoom();

  /**
   * @brief set ip
   * @param newIp new ip number
  */
  void setIp(int newIp);

  /**
   * @brief print all clients
  */
  void printClients();

  /**
   * @brief add a client
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

}
