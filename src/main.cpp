/**
 * @author Justin Nicolas Allard
 * Start of execution, main function
*/

#include <iostream>
#include <unordered_map>

#include "./room/Room.hpp"
#include "./client/Client.hpp"

/**
 * show help information
*/
void showHelp() {
  std::cout <<
  "List of commands:\n\n"
  "\t\'make room\'\t| will prompt for port and IP/host to listen on. Plays audio received from a client.\n\n"
  "\t\'join room\'\t| Will prompt for port and IP/host to join to. A room must be listening at the specified port and host. "
  "If connection is successful, will prompt for name of audio, and path to audio file (must be mp3 format).\n\n";
}

/**
 * Enum class of commands, which we support.
 * These should have a string mapping in `commandMap` below
*/
enum class Command {
  MAKE_ROOM,
  JOIN_ROOM,
  HELP,
  EXIT
};

/**
 * This is a map which maps strings to commands.
 * Allows us to use a switch statement rather than a bunch of if else statements
 * Example: "make room" inputted on the command line maps to Command::MAKE_ROOM, which is handled in the switch in main
 */
static const std::unordered_map<std::string, Command> commandMap = {
  {"make room", Command::MAKE_ROOM},
  {"join room", Command::JOIN_ROOM},
  {"help", Command::HELP},
  {"exit", Command::EXIT}
};

int main() {
  std::string input;
  while (true) {
    std::cout << " >> ";
    std::getline(std::cin, input);
    Command command;
    try {
      command = commandMap.at(input);
    } catch (const std::out_of_range &err){
      std::cout << "Invalid command. Try 'help' for information\n";
      continue;
    }

    // handle all possible commands in this switch
    switch (command) {

      case Command::EXIT:
        return 0;

      case Command::HELP: 
        // show help information, such as list of commands, how to use the application...
        showHelp();
        break;

      case Command::JOIN_ROOM: {
        // "join room" command was entered, so now we treat this user as a client who wants to join a room
        Client client;
        if (client.initializeClient()) {
          client.handleClient();
        }
        break;
      }

      case Command::MAKE_ROOM: {
        // "make room" command was entered, so now we treat this user as a client who wants to make a room
        Room room;
        if (room.initializeRoom()) {
          room.launchRoom();
        }
        break;
      }

      default:
        // this section of code should never be reached
        std::cerr << "Error: Reached default case in main method.\nCommand " << input << " not handled but is in mapCommand\n";
        exit(1);
    }
  }
}
