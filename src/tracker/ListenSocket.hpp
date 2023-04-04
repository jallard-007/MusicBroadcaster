/**
 * @file ListenSocket.hpp
 * @author Tyler Johnson (tjohn73@uwo.ca)
 * @brief This is a ListenSocket used by the tracker. It is a singleton since only one listen socket is needed.
 * It is a singleton of the BaseSocket.
 * @version 0.1
 * @date 2023-03-25
 * 
 * @copyright Copyright (c) 2023
 * 
 */
#pragma once

#include <ctype.h>
#include <stdint.h>

#include "../socket/BaseSocket.hpp"
#include "../socket/IP.hpp"

/**
 * @brief This is our singleton listen socket
 * 
 */
class ListenSocket {

private:

    /**
     * @brief Our internal BaseSocket. It is null when there is nothing in it.
     * 
     */
    static BaseSocket* socket;

public:

    /**
     * @brief Construct a new Listen Socket object. This will do nothing
     * 
     */
    ListenSocket() = default;

    /**
     * @brief Destroy the Listen Socket object. We do not need this
     * 
     */
    ~ListenSocket() = default;
    
    /**
     * @brief This will try to accept connections from the socket.
     * 
     * @return BaseSocket This will return a BaseSocket which is connected to the client, or it may be an empty socket.
     */
    BaseSocket accept();

    /**
     * @brief This will bind the socket to an ip and port
     * 
     * @param addr The IP address to bind to
     * @param port The port to bind to
     * @return true If the socket was bound
     * @return false If the socket failed to bind
     */
    bool bind(const IP& addr, uint16_t port);

    /**
     * @brief This will tell the socket to listen for connections
     * 
     * @param backlog The number of connections to backlog
     * @return true If the socket is listening
     * @return false If the socket failed to be set to listen
     */
    bool listen(int backlog);

    /**
     * @brief Tries to read data off of the socket
     * 
     * @param buf The location of the buffer to use in memory
     * @param size The size of the buffer
     * @return size_t The total number of bytes read
     */
    size_t read(std::byte* buf, const size_t size);

    /**
     * @brief Tries to write data into the socket
     * 
     * @param data The data to write into the socket
     * @param size The size of the data to write to the socket
     * @return size_t The actual number of bytes written
     */
    size_t write(const std::byte* data, const size_t size);

    /**
     * @brief This will close the socket. Since this is a singleton, it will close it for all.
     * 
     */
    void close();

    /**
     * @brief This will check if the socket is open
     * 
     * @return true If there is an acive socket
     * @return false If there is not an active socket
     */
    bool isOpen();

};