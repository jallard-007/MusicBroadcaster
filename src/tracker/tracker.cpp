/**
 * @file main.cpp
 * @author Tyler Johnson (tjohn73@uwo.ca)
 * @brief This is the program which controls the tracker
 * @version 0.1
 * @date 2023-03-07
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#include <iostream>
#include <string>
#include <algorithm>
#include <iterator>
#include <signal.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "../messaging/Commands.hpp"
#include "../messaging/Message.hpp"
#include "ListenSocket.hpp"
#include "RoomEntry.hpp"
#include "TrackerAPI.hpp"

void signalHandler(int signum) {

    std::cout << "Shutting down tracker...";
    std::cout.flush();

    /* Get the listen socket */
    ListenSocket socket;

    /* Close the socket */
    socket.close();

    std::cout << "\nTracker shut down" << std::endl;
    std::cout.flush();

    exit(signum);
}

/**
 * @brief This will add a room to the list of rooms
 * 
 * @param msg The message received from the client
 * @param rooms The vector which holds all the rooms on the tracker 
 */
bool addRoom(const Message& msg, std::vector<RoomEntry>& rooms);

/**
 * @brief This will remove a room from the list of rooms
 * 
 * @param msg The message received from the client
 * @param rooms The vector which holds all the rooms on the tracker
 */
bool removeRoom(const Message& msg, std::vector<RoomEntry>& rooms);

/**
 * @brief Create a message which holds all the rooms based on spec
 * 
 * @param rooms The vector of rooms
 * @return Message The message to send back
 */
Message listRoomsMessage(const std::vector<RoomEntry> rooms);

/**
 * @brief Will try to read the message and find a room. This will return an namesless room if one
 * is not found
 * 
 * @param msg The message sent
 * @param rooms The vector holding all of the rooms
 * @return RoomEntry 
 */
RoomEntry findRoom(const Message& msg, const std::vector<RoomEntry> rooms);

int main(int argc, char* argv[]) {

    if (argc < 2) {
        std::cerr << "Please enter IP with command" << std::endl;
        exit(-1);
    }

    /* Get our IP out of the arguments */
    IP ip = IP(std::string(argv[1]));

    /* Set the signal handler */
    signal(SIGINT, signalHandler);

    /* Do not sync with the stdio */
    std::ios_base::sync_with_stdio(false);

    std::cout << "Starting tracker..." << std::endl;

    std::cout << "Creating socket and binding...";
    std::cout.flush();

    ListenSocket socket;

    /* Bind the socket */
    if (!socket.bind(ip.str(), TRACKER_PORT)) {
        std::cout << std::endl;
        std::cerr << "Failed to bind socket" << std::endl;
        exit(-1);
    }

    std::cout << "\nSocket binding successful" << std::endl;

    std::cout << "Attempting to set socket to listen...";

    /* Set the socket to listen */
    if (!socket.listen(20)) {
        std::cout << std::endl;
        std::cerr << "Failed to set socket to listen" << std::endl;
        socket.close();
        exit(-1);
    }

    std::cout << "\nSocket set to listen" << std::endl;

    std::cout << "Waiting for connections..." << std::endl;

    /* This will hold all of our rooms */
    std::vector<RoomEntry> rooms;

    while (true) {

        /* Try to accept connections */
        BaseSocket client = socket.accept();

        /* Create some space in memory */
        std::byte buf[1024];
        size_t size;
        /* Try to read from the socket */
        if (size = client.read(buf, sizeof(buf)); size > 0) {

            /* Our vector */
            std::vector<std::byte> buffer;
            for (size_t i = SIZE_OF_HEADER; i < size; i++)
                buffer.push_back(buf[i]);

            /* Get the client ip */
            int fileDescriptor = client.getSocketFD();
            struct sockaddr_in clientAddress;
            socklen_t clientAddressLength = sizeof(clientAddress);
            getpeername(fileDescriptor, (struct sockaddr*)&clientAddress, &clientAddressLength);

            /* Copy in the IP to the string */
            std::string ip(inet_ntoa(clientAddress.sin_addr));

            std::cout << "\nMessage received from " << ip << std::endl;
            std::cout << "Message size - " << std::to_string(size) << std::endl;

            /* Create a message from the buffer */
            Message msg{buf};
            msg.setBody(buffer);

            /* Check the command given */
            if (msg.getCommand() == Commands::Command::ADD_ROOM) {

                /* Add the room */
                bool result = addRoom(msg, rooms);

                /* Create the response */
                Message response;
                if (result) {
                    response.setCommand(Commands::Command::GOOD_MSG);
                } else {
                    response.setCommand(Commands::Command::BAD_VALUES);
                }

                /* Send the response */
                const std::vector<std::byte>& responseBytes = response.getMessage();
                client.write(responseBytes.data(), responseBytes.size());
                

            } else if (msg.getCommand() == Commands::Command::REMOVE_ROOM) {

                /* Remove the room */
                bool result = removeRoom(msg, rooms);

                Message response;
                if (result) {
                    response.setCommand(Commands::Command::GOOD_MSG);
                } else {
                    response.setCommand(Commands::Command::BAD_VALUES);
                }

                /* Send the response */
                const std::vector<std::byte>& responseBytes = response.getMessage();
                client.write(responseBytes.data(), responseBytes.size());

            /* Find and list all of the rooms */
            } else if (msg.getCommand() == Commands::Command::LIST_ROOMS) {

                Message msg = listRoomsMessage(rooms);

                /* send the response */
                const std::vector<std::byte>& responseBytes = msg.getMessage();

                std::cout.flush();

                client.write(responseBytes.data(), responseBytes.size());

            } else if (msg.getCommand() == Commands::Command::FIND_ROOM) {
                
                /* Find the room */
                RoomEntry room = findRoom(msg, rooms);

                /* This is our response message */
                Message response;

                /* If there is a room */
                if (room.getName().length() > 0) {
                    
                    response.setCommand(Commands::Command::GOOD_MSG);
                
                } else {

                    response.setCommand(Commands::Command::BAD_VALUES);

                }

                /* Get the message out and then send it through the BaseSocket known as client */
                const std::vector<std::byte>& data = response.getMessage();
                client.write(data.data(), data.size());

            } else {

                std::cout << "Bad command" << std::endl;

                Message response;
                response.setCommand(Commands::Command::RES_NOT_OK);

                /* Send the response */
                const std::vector<std::byte>& responseBytes = response.getMessage();
                client.write(responseBytes.data(), responseBytes.size());

            }

        }

    }

    return 0;

}

bool addRoom(const Message& msg, std::vector<RoomEntry>& rooms) {


    /* Get the room name from the vector of data */
    std::string roomName;

    /* Get the room name */
    auto itr = msg.getBodyBegin();

    while (*itr != (std::byte)'\0' && itr != msg.getBodyEnd()) {
        roomName += (char)*itr;
        itr++;
    }

    /* If we have gone through the entire vector without finding the end of the string */
    if (itr == msg.getBodyEnd()) 
        return false;
    
    if (itr + sizeof(uint32_t) + sizeof(uint16_t) >= msg.getBodyEnd())
        return false;

    /* Get the ip */
    itr++;
    unsigned int ip;
    std::reverse_copy(itr, itr + sizeof(uint32_t), (std::byte *)&ip);

    std::cout << ip << std::endl;

    /* Move the itr over */
    itr += sizeof(uint32_t);

    /* Get the port */
    uint16_t port;
    std::reverse_copy(itr, itr + sizeof(uint16_t), (std::byte *)&port);

    /* Create the room */
    RoomEntry room(roomName, ip, port);

    /* Add the room to the vector */
    rooms.push_back(room);

    /* Tell the calling function we have added the room */
    return true;

}

bool removeRoom(const Message& msg, std::vector<RoomEntry>& rooms) {

    /* This will hold the name of the room */
    std::string name;

    /* Get the name of the room */
    auto itr = msg.getBodyBegin();

    /* Get the name of the room */
    while (*itr != (std::byte)'\0' && itr != msg.getBodyEnd()) {
        itr++;
    }

    /* If we have gone through the entire vector without finding the end of the string */
    if (itr == msg.getBodyEnd()) return false;
    else { /* Copy into string */
        name.reserve(std::distance(msg.getBodyBegin(), itr));
        for_each(msg.getBodyBegin(), itr, [&name](const std::byte& byte) {
        name += (char)byte;
        });
    }

    /* Find the room */
    std::vector<RoomEntry>::iterator roomItr = std::find_if(rooms.begin(), rooms.end(), [&name](const RoomEntry& room) {
        return room.getName() == name;
    });

    /* If we have found the room */
    if (roomItr != rooms.end()) {

        /* Remove the room */
        rooms.erase(roomItr);
        return true;
    }

    /* If we have not found the room */
    return false;

}

/**
 * @brief Create a message to send back with all the rooms in the message
 * 
 * @param rooms The rooms to add to the list
 * @return Message The message to send back
 */
Message listRoomsMessage(const std::vector<RoomEntry> rooms) {

    /* This will hold the message body */
    std::vector<std::byte> body;

    /* Go through each room and add it to the list */
    for (std::vector<RoomEntry>::const_iterator itr = rooms.begin(); itr != rooms.end(); itr++) {

        /* This will hold the message line */
        std::vector<std::byte> temp;

        /* Add the name of the room to the body of the message first */
        for_each(itr->getName().begin(), itr->getName().end(), [&temp](const char character) {
            temp.push_back((std::byte)character);
        });
        temp.push_back((std::byte)'\0');

        /* Push the IP back */
        uint32_t ip = itr->getIP().compressed();
        for (int index = sizeof(uint32_t) - 1; index >= 0; index--) {
            temp.push_back(((std::byte*)(&ip))[index]);
            std::cout << (int)temp.back() << std::endl;
        }

        /* Push the port back */
        uint16_t port = itr->getPort();
        for (int index = sizeof(uint16_t) - 1; index >= 0; index--) {
            temp.push_back(((std::byte*)(&port))[index]);
        }

        /* Move what we created into the body */
        body.insert(body.end(), std::make_move_iterator(temp.begin()), std::make_move_iterator(temp.end()));

    }

    /* Our new msg */
    Message msg;
    msg.setCommand(Commands::Command::GOOD_MSG);
    msg.setBody(body);

    /* Move the message */
    return msg;  

}

/**
 * @brief Will try to read the message and find a room. This will return a nameless room if one
 * is not found
 * 
 * @param msg The message sent
 * @param rooms The vector holding all of the rooms
 * @return RoomEntry 
 */
RoomEntry findRoom(const Message& msg, const std::vector<RoomEntry> rooms) {

    /* First, we need to get the name out of the body of the message */
    auto itr = msg.getBodyBegin();
    while (*itr != (std::byte)'\0' && itr != msg.getBodyEnd()) { /* Loop through and try to load in the name of the room */
        itr++;
    }

    /* This will hold the name of the room */
    std::string name;
    name.reserve(msg.getBodyEnd() - msg.getBodyBegin());

    /* If we are still at the beginning (including the '\0' element, so we look for distances >1)*/
    if (std::distance(msg.getBodyBegin(), itr) > 1) return RoomEntry();
    else { /* Otherwise, we then get the name from the body */

        /* Load each byte of the vector into the string */
        for_each(msg.getBodyBegin(), itr, [&name](std::byte Byte) {
            name += (char)Byte;
        });

    }

    /* Now that we have the name of the room, we go searching for it */
    RoomEntry room;
    for (std::vector<RoomEntry>::const_iterator rIt = rooms.begin(); rIt != rooms.end(); rIt++) {
        if (rIt->getName() == name) {
            room = *rIt;
        }
    }

    return room;

}