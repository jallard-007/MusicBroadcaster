#include <iostream>
#include <string>
#include "CLInput.hpp"

uint16_t getPort() {
  while (1) { // loop until valid number
    std::cout << "Enter a port number\n >> ";
    std::string input;
    std::getline(std::cin, input);
    char *endPtr;
    const uint16_t port = (uint16_t)strtol(input.c_str(), &endPtr, 10);
    if (*endPtr == '\0') {
      return port;
    }
    std::cout << "Error: not a valid port number\n";
  }
}

void getHost(std::string &input) {
  while (1) { // loop until valid number
    std::cout << "Enter a host\n >> ";
    std::getline(std::cin, input);
    if (input != "") {
      return;
    }
    std::cout << "Error: not a valid host\n";
  }
}
