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
} PipeData_t;

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
   * Pipe to communicate from child threads to parent thread
  */
  int threadRecvPipe[2];

  /**
   * Pipe to communicate from child threads to parent thread
  */
  int threadSendPipe[2];

  /**
   * Pipe to communicate from child threads to parent thread
  */
  int threadWaitAudioPipe[2];

  /**
   * When the current song started playing to the nearest second
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
   * @brief Removes an entry from the queue, sends all clients a Command::REMOVE_QUEUE_ENTRY, calls Room::attemptPlayNext
  */
  void handleRemoveQueueEntry(MusicStorageEntry *);

  /**
   * @brief Handles connection requests
  */
  void handleConnectionRequests();

  /**
   * @brief Handles external client's request of SONG_DATA
   * @details Threaded function which reads sizeOfFile bytes from p_client
   * @param p_client a pointer to the client in which to read data from
   * @param sizeOfFile the size of the aud
   */
  void handleClientReqSongData_threaded(room::Client *p_client, uint32_t sizeOfFile);

  /**
   * @brief Handles external client's request of REQ_ADD_TO_QUEUE
  */
  void handleClientReqAddQueue(room::Client &client);

  /**
   * @brief Handles incoming messages from the client.
   * @param client reference to client object
   * @returns false if the client should be removed, true otherwise
  */
  bool handleClientRequests(room::Client& client);

  /**
   * @brief Helper to Room::handleStdinAddSong
   * @details Threaded function, allows the room to continue managing requests from other clients,
   * while still getting the correct input from the room host
   * @param p_entry a pointer to an entry in the queue
  */
  void handleStdinAddSongHelper_threaded(MusicStorageEntry *p_entry);

  /**
   * @brief Handles the stdin 'add song' command
  */
  void handleStdinAddSong();

  /**
   * @brief Handles all stdin input
   * @details Generally, just reads in one line from stdin and calls other,
   * more focused functions to handle specific commands
  */
  int handleStdinCommands();

  /**
   * @brief Handles when a thread finishes receiving song data
   * @details More specifically, this is called in the main thread when there is data to be read from the threadRecvPipe
  */
  void processThreadFinishedReceiving();

  /**
   * @brief Handles when a thread finishes sending song data
   * @details More specifically, this is called in the main thread when there is data to be read from the threadSendPipe
  */
  void processThreadFinishedSending();

  /**
   * @brief Sends song data to a specific client
   * 
   * @param audio a shared Music object in which the data to be sent is stored
   * @param p_queue a pointer to the MusicStorageEntry object which corresponds to the song being sent
   * @param queuePosition the MusicStorageEntry's position in the queue
   * @param p_client a pointer to the room::Client object to send the data to
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
  void sendSongToAllClients(const PipeData_t &);

  /**
   * @brief waits for audio to stop playing, then notifies the main thread via Room::threadWaitAudioPipe
  */
  void waitOnAudio_threaded();

  /**
   * @brief Attempts to play the next song, cancels if something is already playing
  */
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
  [[nodiscard]] int getIp() const;
  [[nodiscard]] const std::string &getName() const;
  [[nodiscard]] const std::list<room::Client> &getClients() const;
};

}
