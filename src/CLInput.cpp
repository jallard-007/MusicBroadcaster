#include <iostream>
#include <string>

#include "CLInput.hpp"

uint16_t getPort() {
  while (true) { // loop until valid number
    std::cout << "Enter a port number:\n >> ";
    std::string input;
    std::getline(std::cin, input);
    char *endPtr;
    const auto port = (uint16_t)strtol(input.c_str(), &endPtr, 10);
    if (*endPtr == '\0') {
      return port;
    }
    std::cout << "Error: not a valid port number\n";
  }
}

void getHost(std::string &input) {
  while (true) { // loop until some input
    std::cout << "Enter a host:\n >> ";
    std::getline(std::cin, input);
    if (!input.empty()) {
      return;
    }
    std::cout << "Error: not a valid host\n";
  }
}

void getMP3FilePath(Music &m) {
  std::string input;
  while (true) {
    std::cout << "Enter file path (-1 to cancel):\n >> ";
    std::getline(std::cin, input);
    if (input == "-1") {
      m.setPath(input);
      return;
    }
    if (input.length() < 5 || input.substr(input.length() - 4) != ".mp3") {
      std::cerr << "Error: not a valid mp3 file\n";
      continue;
    }
    m.setPath(input);
    if (!m.validateFileAtPath()) {
      continue;
    }
    return;
  }
}
