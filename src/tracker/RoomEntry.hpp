/**
 * @file RoomEntry.hpp
 * @author Tyler Johnson (tjohn73@uwo.ca)
 * @brief This will represent a room entry in the tracker
 * @version 1.1
 * @date 2023-03-25
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#pragma once

#include <string>
#include <stdint.h>

#include "../socket/IP.hpp"

class RoomEntry {

private:
    
    /**
    * @brief The name of the room
    * 
    */
    std::string name;

    /**
    * @brief The ip of the room
    * 
    */
    IP ip;

    /**
    * @brief The port of the room
    * 
    */
    uint16_t port;

public:

    /**
    * @brief Construct a new Room Entry object
    * 
    * @param name The name of the room
    * @param ip The ip of the room
    * @param port The port of the room
    */
    RoomEntry(const std::string& name, const IP& ip, uint16_t port);

    /**
     * @brief Default construction. Will set string to nothing, and IP and port to 0
     * 
     */
    RoomEntry();

    /**
    * @brief Destroy the Room Entry object
    * 
    */
    ~RoomEntry() = default;



    /**
     * @brief Sets the name of the room
     * 
     * @param name The name to set the room to
     */
    void setName(const std::string& name) noexcept;

    /**
     * @brief Sets the ip of the room
     * 
     * @param ip The IP of the room in byte format
     */
    void setIP(const IP& ip) noexcept;

    /**
     * @brief Sets the port of the room
     * 
     * @param port The 2 byte port of the room
     */
    void setPort(uint16_t port) noexcept;


    /**
    * @brief Get the Name object
    * 
    * @return std::string The name of the room
    */
    const std::string& getName() const noexcept;

    /**
    * @brief Get the Ip object
    * 
    * @return IP The ip of the room
    */
    const IP& getIP() const noexcept;

    /**
    * @brief Get the Port object
    * 
    * @return uin16_t The port of the room
    */
    uint16_t getPort() const noexcept;


};