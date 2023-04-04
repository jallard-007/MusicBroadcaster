#pragma once

#include <string>
#include <iostream>

#include "./music/Music.hpp"

/**
 * Functions to get specific input via the command line
*/

/**
 * @brief Get the port of the room
 * 
 * @return uint16_t The port of the room
 */
uint16_t getPort();

/**
 * @brief Get the IP of the room
 * 
 * @param input The IP string to store the result in
 */
void getHost(std::string &input);

/**
 * @brief Get mp3 file path
 */
void getMP3FilePath(Music &m);
