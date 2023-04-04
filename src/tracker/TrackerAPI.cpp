/**
 * @file TrackerAPI.cpp
 * @author Tyler Johnson (tjohn73@uwo.ca)
 * @brief This is the implementation of the TrackerAPI class
 * @version 0.1
 * @date 2023-03-10
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#include "TrackerAPI.hpp"

bool TrackerAPI::add(const std::string& name, const IP& ip, uint16_t port) {

    /* Our message */
    Message msg;

    msg.setCommand(Commands::Command::ADD_ROOM);
    
    /* Create the body for the message */
    std::vector<std::byte> body;
    
    /* Insert into the vector */
    for (std::string::const_iterator itr = name.begin(); itr != name.end(); itr++)
        body.push_back((std::byte)*itr);

    body.push_back((std::byte)'\0');


    /* Push back the ip and port. This has to be added backwards to keep the order of the bytes */
    int index;
    uint32_t addr = ip.compressed(); /* Our IP address */
    for (index = sizeof(uint32_t) - 1; index >= 0; index--) {
        body.push_back(((std::byte *)&addr)[index]);
    }

    for (index = sizeof(uint16_t) - 1; index >= 0; index--)
        body.push_back(((std::byte *)&port)[index]);

    msg.setBody(body);

    /* Get the formatted msg out */
    const std::vector<std::byte> &formatted_msg = msg.getMessage();

    /* This will be our socket */
    BaseSocket socket;

    /* Connect to the tracker */
    if (!socket.connect(TRACKER_NAME, TRACKER_PORT))
        return false;

    /* Write to the socket */
    if (!socket.write(formatted_msg.data(), formatted_msg.size()))
        return false;

    /* Create space and read out the data */
    std::vector<std::byte> buf;
    std::byte buffer[256];
    size_t size;
    if (size = socket.read(buffer, sizeof(buffer)); size <= 0)
        return false;

    for (unsigned int i = 0; i < size; i++)
        buf.push_back(buffer[i]);

    /* Parse the result */
    Message result;
    result.setBody(buf);

    return Message::good(result);

}

/* Deregister the name of the room */
bool TrackerAPI::remove(const std::string& name) {

    /* This will be our message */
    Message msg;
    /* Add the remove room command */
    msg.setCommand(Commands::Command::REMOVE_ROOM);

    /* The body for our message */
    std::vector<std::byte> body;
    
    /* Insert into the vector */
    for (std::string::const_iterator itr = name.begin(); itr != name.end(); itr++)
        body.push_back((std::byte)*itr);
    
    body.push_back((std::byte)'\0');

    msg.setBody(body);

    const std::vector<std::byte> &formatted_msg = msg.getMessage();

    /* Create the socket and then send the data */
    BaseSocket socket;
    if (!socket.connect(TRACKER_NAME, TRACKER_PORT))
        return false;

    /* Write to the socket */
    if (!socket.write(formatted_msg.data(), formatted_msg.size()))
        return false;

    std::vector<std::byte> buf;
    std::byte* buffer = new std::byte[256];
    size_t size;
    if (size = socket.read(buffer, sizeof(buffer)); size <= 0)
        return false;

    for (size_t i = 0; i < size; i++)
        buf.push_back(buffer[i]);

    /* Parse the message */
    Message result;
    result.setBody(buf);

    return Message::good(result);

}

/* Lookup the name of the room */
RoomEntry TrackerAPI::lookup(const std::string& name) {

  /* Start with our message again */
  Message msg;

  msg.setCommand(Commands::Command::FIND_ROOM);

  /* Add the name of the room to lookup */
  std::vector<std::byte> body;

  /* Insert into the vector */
  for (char itr : name)
    body.push_back((std::byte) itr);

  body.push_back((std::byte) '\0');

  /* Add the body to the message */
  msg.setBody(body);

  /* Get the message */
  const std::vector<std::byte> &formatted_msg = msg.getMessage();

  /* Connect to the tracker */
  BaseSocket socket{};
  if (!socket.connect(TRACKER_NAME, TRACKER_PORT)) {
    return RoomEntry();
  }

    /* Send the data */
    if (!socket.write(formatted_msg.data(), formatted_msg.size()))
        return RoomEntry();

    /* Get the response */
    std::vector<std::byte> buf;
    std::byte* buffer = new std::byte[256];
    size_t size;
    if (size = socket.read(buffer, sizeof(buffer)); size <= 0)
        return RoomEntry();

    for (size_t i = SIZE_OF_HEADER; i < size; i++)
        buf.push_back(buffer[i]);

    Message result(buffer);
    result.setBody(buf);

    /* read out the body if it is good (body should be no bigger or smaller than 6 bytes)*/
    if (Message::good(result) && msg.getBodySize() == sizeof(uint32_t) + sizeof(uint16_t)) {

        /* Get the body */
        const std::byte *resulting_body = msg.getMessage().data() + SIZE_OF_HEADER;
        
        /* Load in the ip and port */
        uint32_t ip;
        uint16_t port;
        size_t index;

        for (index = 0; index < sizeof(uint32_t); index++)
            ((std::byte *)&ip)[index] = resulting_body[index];

        size_t port_index = 0;
        for (index = sizeof(uint32_t) + 1; index < sizeof(uint32_t) + sizeof(uint16_t); index++)
            ((std::byte *)&port)[port_index++] = resulting_body[index];

        /* Return the ip and port */
        return RoomEntry(name, IP{ip}, port);

    }

    /* If we reach here with nothing left, there is nothing more we can do */
    return RoomEntry();

}


std::vector<RoomEntry> TrackerAPI::list() {

    /* This will be our message */
    Message msg; 

    /* Set the command to get the list of rooms */
    msg.setCommand(Commands::Command::LIST_ROOMS);

    /* Format the message */
    const std::vector<std::byte> &formatted_msg = msg.getMessage();

    /* This will hold our rooms */
    std::vector<RoomEntry> room_info;

    /* Connect our socket */
    BaseSocket socket;
    if (!socket.connect(TRACKER_NAME, TRACKER_PORT))
        return room_info;

    /* Write our data out */
    if (!socket.write(formatted_msg.data(), formatted_msg.size()))
        return room_info;

    /* Read in the new data */
    std::vector<std::byte> buf;
    std::byte buffer[512];
    size_t size;
    if (size = socket.read(buffer, sizeof(buffer)); size <= 0)
        return room_info;

    for (size_t i = SIZE_OF_HEADER; i < size; i++)
        buf.push_back(buffer[i]);

    /* Parse the message */
    Message result{buffer};
    result.setBody(buf);
    
    /* This keeps us in the body bounds */
    std::vector<std::byte>::const_iterator itr = result.getMessage().begin();
    itr += SIZE_OF_HEADER;
    
    /* Keeps us in the bounds */
    while (itr != result.getMessage().end()) {

        /* The rooms information */
        std::string name;
        uint32_t ip;
        uint16_t port;

        /* Get the name out of the room */
        while (itr != result.getMessage().end() && *itr != (std::byte)'\0') {

            /* Read in the character */
            name += (char)*itr;
            itr++;

        }

        /* If we didn't read in a single character */
        if (name.size() == 0) {
            return room_info;
        }
        
        /* Shift over */
        itr++;

        /* Get the next 4 bytes for the ip */
        int index;
        for (index = (int)sizeof(uint32_t) - 1; itr != result.getMessage().end() && index >= 0; index--) {
            ((std::byte *)&ip)[index] = *itr++;
        }
        

        /* If we didn't read in all the bytes */
        if (index != -1) {
            return room_info;
        }

        /* Get out the next 2 bytes */
        for (index = (int)sizeof(uint16_t) - 1; itr != result.getMessage().end() && index >= 0; index--) {
            ((std::byte *)&port)[index] = *itr++;
        }

        /* If we didn't read in enough bytes */
        if (index != -1) 
            return room_info;
        

        /* Push back the information */
        room_info.emplace_back(name, IP(ip), port);

        /* Shift over to check the next name */
        itr++;

    }

    /* Return what we got */
    return room_info;

}

int TrackerAPI::associated() {

    /* Again, we need a message object */
    Message msg;

    /* Set the command to count the rooms that the tracker knows */
    msg.setCommand(Commands::Command::COUNT_ROOMS);

    /* We don't need a body */
    const std::vector<std::byte>& formatted_msg = msg.getMessage();

    /* Create the socket */
    BaseSocket socket;

    /* Connect the socket */
    if (!socket.connect(TRACKER_NAME, TRACKER_PORT))
        return -1;

    /* Write to the socket */
    if (!socket.write(formatted_msg.data(), formatted_msg.size()))
        return -1;

    std::vector<std::byte> buf;
    std::byte* buffer = new std::byte[256];
    size_t size;

    if (size = socket.read(buffer, sizeof(buffer)); size <= 0)
        return -1;

    for (size_t i = SIZE_OF_HEADER; i < size; i++)
        buf.push_back(buffer[i]);

    Message result{buffer};
    result.setBody(buf);

    /* Use index and iteration method */
    size_t index;
    int count;
    std::vector<std::byte>::const_iterator itr = result.getMessage().begin();
    itr += SIZE_OF_HEADER;
    for (index = 0; itr != result.getMessage().begin() && index < sizeof(int); index++)
        ((std::byte *)&count)[index] = *itr++;

    /* Quickly check if indexing errors. For example, if we didn't read in all the bytes */
    if (index != sizeof(int) - 1)
        return -1;
    else
        return count;

}
