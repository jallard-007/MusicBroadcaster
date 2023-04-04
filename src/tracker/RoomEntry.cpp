/**
 * @file RoomEntry.cpp
 * @author Tyler Johnson (tjohn73@uwo.ca)
 * @brief This is the implementation of the RoomEntry class
 * @version 1.1
 * @date 2023-03-25
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#include "RoomEntry.hpp"

/**
 * @brief Construct a new Room Entry object
 * 
 * @param name The name of the room
 * @param ip The IP of the room
 * @param port The port of the room
 */
RoomEntry::RoomEntry(const std::string& name, const IP& ip, unsigned short port) {
    this->name = name;
    this->ip = ip;
    this->port = port;
}

/**
 * @brief Default construction. Will set string to nothing, and IP and port to 0
 * 
 */
RoomEntry::RoomEntry() {
    this->name = std::string();
    this->ip = IP();
    this->port = (unsigned short)0;
}

/**
 * @brief Sets a new name for the room
 * 
 * @param name The new name to give the room
 */
void RoomEntry::setName(const std::string& name) noexcept {
    this->name = name;
}

/**
 * @brief Sets a new ip for the room
 * 
 * @param ip The new ip to give the room
 */
void RoomEntry::setIP(const IP& ip) noexcept {
    this->ip = ip;
}

/**
 * @brief Sets a new port for the room
 * 
 * @param port The new port to give the room
 */
void RoomEntry::setPort(uint16_t port) noexcept{
    this->port = port;
}

/**
 * @brief Gets the name of the room
 * 
 * @return std::string The name of the room
 */
const std::string& RoomEntry::getName() const noexcept {
    return this->name;
}

/**
 * @brief Gets the ip of the room
 * 
 * @return unsigned int The ip of the room
 */
const IP& RoomEntry::getIP() const noexcept {
    return this->ip;
}

/**
 * @brief Gets the port of the room
 * 
 * @return unsigned short The port of the room
 */
uint16_t RoomEntry::getPort() const noexcept {
    return this->port;
}