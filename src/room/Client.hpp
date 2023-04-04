/**
 * @author Justin Nicolas Allard
 * Header file for server side client class
 */

#pragma once

#include <thread>
#include <utility>
#include "../music/MusicStorage.hpp"
#include "../socket/ThreadSafeSocket.hpp"

/**
 * @brief Namespace used to differentiate room (server side) client from client side client class
 * This is the room client
 */
namespace room { 

   /**
   * @brief Server side client class. Used as an entry into the rooms list of connected clients
   * Doesn't actually do anything, just holds information
   */
  class Client {
  public:
    int entriesTillSynced;
    MusicStorageEntry *p_entry{};

  private:

    /**
     * name of client
    */
    std::string name;

    /**
     * socket associated with the client
    */
    ThreadSafeSocket socket;

  public:

    /**
     * @brief Construct a new Client object
     * 
     */
    Client();

    /**
     * @brief Constructor which takes the name of the client along with the socket associated with the client
     * @param name name of client
     * @param socket socket associated with this client
     */
    Client(std::string name, ThreadSafeSocket &&socket);

    /**
     * @brief Move constructor
     */
    Client(Client &&moved) noexcept;

    /**
     * @brief Delete the copy constructor. This is because BaseSocket closes the file descriptor in its destructor so we cannot copy
     */
    Client(const Client &) = delete;

    /**
     * @brief the equality operator
     */
    bool operator==(const room::Client &rhs) const;

    /**
     * @brief Getter for name field
     */
    [[nodiscard]] const std::string &getName() const;

    /**
     * @brief Getter for socket
     */
    ThreadSafeSocket &getSocket();

  };

}
