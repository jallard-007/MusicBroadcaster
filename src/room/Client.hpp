/**
 * @author Justin Nicolas Allard
 * Header file for server side client class
*/

#ifndef CLIENT_ENTRY_H
#define CLIENT_ENTRY_H

#include <thread>
#include "../socket/ThreadSafeSocket.hpp"

/**
 * Namespace used to differentiate room (server side) client from client side client class
 * This is the room client
*/
namespace room { 

  /**
   * Server side client class. Used as an entry into the rooms list of connected clients
   * Doesn't actually do anything, just holds information
  */
  class Client {
  public:
    int sendingCount;
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
    Client();

    /**
     * Constructor which takes the name of the client along with the socket associated with the client
     * @param name name of client
     * @param socket socket associated with this client
    */
    Client(std::string name, ThreadSafeSocket &&socket);

    /**
     * Move constructor
    */
    Client(Client &&moved) noexcept;

    /**
     * Delete the copy constructor. This is because BaseSocket closes the file descriptor in its destructor so we cannot copy
    */
    Client(const Client &) = delete;

    /**
     * the equality operator
    */
    bool operator==(const room::Client &rhs) const;

    /**
     * Getter for name field
    */
    [[nodiscard]] const std::string &getName() const;

    /**
     * Getter for socket
    */
    ThreadSafeSocket &getSocket();

  };

}

#endif
