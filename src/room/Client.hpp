/**
 * \author Justin Nicolas Allard
 * Header file for server side client class
*/

#ifndef CLIENT_ENTRY_H
#define CLIENT_ENTRY_H

#include <thread>
#include "../socket/BaseSocket.hpp"

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
  private:

    /**
     * name of client
    */
    std::string name;

    /**
     * socket associated with the client
    */
    BaseSocket socket;

    /**
     * thread associated with this client
    */
    std::thread thread;

  public:
    Client();

    /**
     * Constructor which takes the name of the client along with the socket associated with the client
     * \param name name of client
     * \param socketFD file descriptor of socket associated with this client
    */
    Client(const std::string &name, int socketFD);

    /**
     * Move constructor
    */
    Client(Client &&moved);


    /**
     * Getter for name field
    */
    const std::string &getName() const;

    /**
     * Getter for thread pointer
    */
    const std::thread &getThread() const;

    /**
     * Setter for thread pointer
    */
    void setThread(std::thread &&movedThread);

    /**
     * waits for the thread to finish execution
    */
    void waitForThread();

    /**
     * Getter for socket
    */
    BaseSocket &getSocket();
  };

}

#endif
