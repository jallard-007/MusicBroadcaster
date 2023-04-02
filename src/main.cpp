/**
 * @author Justin Nicolas Allard
 * Start of execution, main function
 * local ip: ipconfig getifaddr en0
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
  "'faq'       | Answers to frequently asked questions\n\n"
  "'help'      | List commands and what they do.\n\n"
  "'exit'      | Exit the program.\n\n"
  "'make room' | Make a room. Will prompt for port and IP/host to listen on.\n\n"
  "'join room' | Join a room. Will prompt for port and IP/host to join.\n\n";
}

void showFAQ() {
  std::cout <<
  "What does this program do?\n"
  "  This program allows you to share and listen to music with your friends, from anywhere in the world.\n\n"
  "How do I use this program?\n"
  "  Start by having one person make a room using the 'make room' command.\n"
  "  Then have all your friends join by using the 'join room' command.\n"
  "  After successfully making/joining a room, use the 'faq' command again for further information\n\n"
  ;
}

/**
 * Enum class of commands, which we support.
 * These should have a string mapping in `commandMap` below
 * FAQ, HELP, and EXIT should be available in all sections
*/
enum class Command {
  FAQ,
  HELP,
  EXIT,
  MAKE_ROOM,
  JOIN_ROOM
};

/**
 * This is a map which maps strings to commands.
 * Allows us to use a switch statement rather than a bunch of if else statements
 * Example: "make room" inputted on the command line maps to Command::MAKE_ROOM, which is handled in the switch in main
 */
const std::unordered_map<std::string, Command> commandMap = {
  {"faq", Command::FAQ},
  {"help", Command::HELP},
  {"exit", Command::EXIT},
  {"make room", Command::MAKE_ROOM},
  {"join room", Command::JOIN_ROOM}
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
      std::cout << "Invalid command. Try 'help' for a list of commands\n";
      continue;
    }

    // handle all possible commands in this switch
    switch (command) {

      case Command::FAQ:
        showFAQ();
        break;

      case Command::HELP: 
        showHelp();
        break;

      case Command::EXIT:
        return 0;
      
      case Command::MAKE_ROOM: {
        // "make room" command was entered, so now we treat this user as a client who wants to make a room
        room::Room room;
        if (room.initializeRoom()) {
          if (!room.launchRoom()) {
            return 0;
          }
        }
        break;
      }

      case Command::JOIN_ROOM: {
        // "join room" command was entered, so now we treat this user as a client who wants to join a room
        clnt::Client client;
        if (client.initializeClient()) {
          if (!client.handleClient()) {
            return 0;
          }
        }
        break;
      }

      default:
        // this section of code should never be reached
        std::cerr << "Error: Reached default case in main\nCommand " << input << " not handled but is in mapCommand\n";
        exit(1);
    }
  }
}

/* tests

  const std::regex regEx{"/tmp/musicBroadcaster_[-a-zA-Z0-9._]{6}"};
  std::vector<std::string> strs = {
    "/tmp/musicBroadcaster_adscfg",
    "/tmp/musicBroadcaster_a.s.fg",
    "/tmp/musicBroadcaster_a-s-g_",

    "/tmp/musicBroadcaster_ad234fg",
    "/usr/tmp/musicBroadcaster_adsfg",
    "/usr/include",
  };
  for (std::string &sr : strs) {
    if (std::regex_match(sr, regEx)) {
      std::cout << "match: " << sr << '\n';
    } else {
      std::cout << "not match: " << sr << '\n';
    }
  }



*/
