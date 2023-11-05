/**
 * @file Commands.hpp
 * @author Justin Nicolas Allard
 * @brief This will handle creating commands for the room or tracker
 */

#pragma once

namespace Commands {

/**
 * @brief This represents the commands that can be sent to the room or tracker
 * 
 */
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
    CANCEL_REQ_ADD_TO_QUEUE, /* client asks room if it can queue a song */
                      /* example: REQ_ADD_TO_QUEUE <options byte> <4 bytes size of whole audio file> */
    RES_ADD_TO_QUEUE_OK,
    RES_ADD_TO_QUEUE_NOT_OK,

    REMOVE_QUEUE_ENTRY,
    /* message contains song data
     * should only be sent if received an ok after REQ_ADD_TO_QUEUE
     * example: SONG_DATA <option byte> <4 bytes size of body> <body>
     * can also be sent by room to a client at anytime
    */
    SONG_DATA,

    /**
     * play the next song in queue, sent from server to clients
    */
    PLAY_NEXT,

    /**
     * confirmation by server
    */
    RES_OK,
    RES_NOT_OK,

    /**
     * response by client to room saying received all song data
    */
    RECV_OK,

    GOOD_MSG, /* Says the return was good */
    BAD_FORMAT, /* Says the format was bad */
    BAD_VALUES /* Values given to the recipient were bad or did not make sense */
};


/* These will be our command specific options */
/* They will start with the command name then the option */
#define JOIN_NAME (std::byte)1 /* With this option, a name should be in the body as a null terminated string */

}
