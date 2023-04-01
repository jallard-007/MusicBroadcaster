#ifndef CL_INPUT_H
#define CL_INPUT_H

#include <string>

#include "./music/Music.hpp"

/**
 * Functions to get specific input via the command line
*/

uint16_t getPort();
void getHost(std::string &input);
void getMP3FilePath(Music &m);

#endif // CL_INPUT_H
