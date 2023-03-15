#pragma once

#include "Message.hpp"

namespace Commands {

enum class Command : std::underlying_type_t<std::byte> {
    JOIN, /* Will be used to join the server */
    LEAVE, /* Tells the room that the client is leaving */
    CLIENTS, /* Tells a room to list out the clients in the same room */
    CHAT, /* Tries to chat with the client*/
    ADD_ROOM, /* Tells the tracker to add a room, the body should be */
              /* This should be the null terminated string, 4 bytes more, and then 2 bytes for the port */
    REMOVE_ROOM, /* Tells the tracker to remove a room */
                 /* Use a null terminated string in the body */
    PING_ROOM, /* Tells the tracker to ping a room to see if it is still available */
               /* No body is need as this should be sent to the room */
    LIST_ROOMS, /* This tells the tracker to list out the registered rooms */
                /* There is no body needed for this */
    COUNT_ROOMS, /* Tells the room to count the number of registered rooms */
                 /* No body need for this */
    FIND_ROOM, /* Tries to find the room */

    REQ_ADD_TO_QUEUE, /* client asks room if it can queue a song */
                      /* example: REQ_ADD_TO_QUEUE <options byte> <4 bytes size of whole audio file> */

    /* message contains song data
    should only be sent if received an ok after REQ_ADD_TO_QUEUE
    example: SONG_DATA <option byte> <4 bytes size of body> <body> */
    SONG_DATA,
    RES_OK,
    RES_NOT_OK,

    GOOD_MSG, /* Says the return was good */
    BAD_FORMAT, /* Says the format was bad */
    BAD_VALUES /* Values given to the recipient were bad or did not make sense */
};


/* These will be our command specific options */
/* They will start with the command name then the option */
#define JOIN_NAME (std::byte)1 /* With this option, a name should be in the body as a null terminated string */


    /**
     * @brief Reads if a message was good
     * 
     * @param msg The returned message 
     * @return true If the returned message was good
     * @return false The message is therefore bad
     */
    inline bool good(const Message& msg);

    /**
     * @brief Read if the message was formatted badly. Needs the return message
     * 
     * @param msg The returned message
     * @return true If the message the was recently sent was badly formatted
     * @return false The message format was okay
     */
    inline bool bad_format(const Message& msg);

    /**
     * @brief Reads the returned message to tell if the values in the message were bad
     * 
     * @param msg The returned message
     * @return true If the values are bad
     * @return false Then the values given in the recent message are good
     */
    inline bool bad_values(const Message& msg);

}

