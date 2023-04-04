/**
 * @file TrackerAPI.hpp
 * @author Tyler Johnson (tjohn73@uwo.ca)
 * @brief Helps the program communicate with the tracker service
 * @version 0.1
 * @date 2023-03-10
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#pragma once

#include <vector>
#include <string>
#include <tuple>
#include <sstream>

#include "../messaging/Message.hpp"
#include "../messaging/Commands.hpp"
#include "../socket/BaseSocket.hpp"
#include "../socket/IP.hpp"
#include "RoomEntry.hpp"

#define TRACKER_NAME "westernradio.ddns.net"
#define TRACKER_PORT 55520

/**
 * @brief This helps out the rest of the program communicate with the tracker.
 * It allows for searching up registered rooms, adding them, and getting a list to them.
 * 
 */
class TrackerAPI {

public:

    TrackerAPI() = delete;
    ~TrackerAPI() = delete;

    /**
     * @brief Will register a name to the ip and port
     * 
     * @param name The name to register the room to
     * @param ip IP to register the room to
     * @param port The port to use
     * @return bool Will only fail if the room name is already in use
     */
    static bool add(const std::string& name, const IP& ip, uint16_t port);

    /**
     * @brief Deregister the room with this name
     * 
     * @param name The name of the room
     * @return bool If the name was removed.
     */
    static bool remove(const std::string& name);

    /**
     * @brief Will search for a name and return the value in a tuple
     * 
     * @param name The name to search for
     * @return std::tuple<std::string, uint16_t> IP and Port stored in 4 byes and 2 bytes 
     */
    static RoomEntry lookup(const std::string& name);

    /**
     * @brief Will return a vector of ALL the rooms
     * 
     * @return std::vector<std::tuple<std::string, std::string, uint16_t>> A vector of tuples which contain the room name, ip, and port. 0s are returned if failed
     */
    static std::vector<RoomEntry> list();

    /**
     * @brief Returns the number of rooms registered
     * 
     * @return int The number of rooms registered. -1 if connection fails
     */
    static int associated();

};

