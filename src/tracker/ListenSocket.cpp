/**
 * @file ListenSocket.cpp
 * @author Tyler Johnson (tjohn73@uwo.ca)
 * @brief This is the actual implementation of the ListenSocket class
 * @version 0.1
 * @date 2023-03-25
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#include "ListenSocket.hpp"

/**
 * @brief Set our socket to null
 * 
 */
BaseSocket* ListenSocket::socket = nullptr;

/**
 * @brief Here we will try to accept a connection
 * 
 * @return BaseSocket A BaseSocket. This may be empty though.
 */
BaseSocket ListenSocket::accept() {

    /* If we have a socket to work with */
    if (this->socket != nullptr) {

        /* Try to accept a connection */
        return this->socket->accept();

    } else {

        /* Return this silly socket */
        BaseSocket socket;
        return socket;

    }

}

/**
 * @brief Here we will try to bind to a socket
 * 
 * @param addr The ip to bind to
 * @param port The port to bind to
 * @return true If the bind was successful
 * @return false If the bind was unsuccessful
 */
bool ListenSocket::bind(const IP& addr, uint16_t port) {

    /* If we have a working socket, then try to bind to the socket */
    if (this->socket != nullptr) {

        /* Try to bind */
        return this->socket->bind(addr.str(), port);

    } else {

        /* Create the socket then try to bind */
        this->socket = new BaseSocket();
        return this->socket->bind(addr.str(), port);
        
    }
    
}

/**
 * @brief Here we will try to listen on the socket
 * 
 * @param backlog The number of connections to backlog while dealing with another one
 * @return true If the listen was succuessful
 * @return false If the listen failed
 */
bool ListenSocket::listen(int backlog) {

    /* If we have a working socket, then try to listen to the socket */
    if (this->socket != nullptr) {

        /* Try to listen */
        return this->socket->listen(backlog);

    } else {

        /* Cannot listen */
        return false;
    }

}

/**
 * @brief Here we will try to read from the socket
 * 
 * @param buf The buffer to write into
 * @param size The size of the buffer
 * @return size_t The number of bytes read
 */
size_t ListenSocket::read(std::byte* buf, const size_t size) {

    /* try to read from the socket */
    if (this->socket != nullptr) {

        return this->socket->read(buf, size);

    } else {

        return 0;
    }

}

/**
 * @brief Here we will try to write to the socket
 * 
 * @param buf The buffer to write from
 * @param size The size of the buffer
 * @return size_t The number of bytes written
 */
size_t ListenSocket::write(const std::byte* buf, const size_t size) {

    /* try to write to the socket */
    if (this->socket != nullptr) {

        return this->socket->write(buf, size);

    } else {

        return 0;
    }

}

/**
 * @brief Here we will try to close the socket
 * 
 */
void ListenSocket::close() {

    /* If we have a socket, then close it */
    if (this->socket != nullptr) {
        /* The destructor will close the socket */
        delete this->socket;
        this->socket = nullptr;
    }

}

/**
 * @brief Here we will check if the socket is open
 * 
 * @return true If the socket is active
 * @return false If there is no active socket
 */
bool ListenSocket::isOpen() {
    return this->socket != nullptr;
}