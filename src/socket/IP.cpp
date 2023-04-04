/**
 * @file IP.cpp
 * @author Tyler Johnson (tjohn73@uwo.ca)
 * @brief This file implements the IP class
 * @version 0.1
 * @date 2023-03-30
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#include "IP.hpp"

/**
 * @brief Construct a new IP object using the uint32_t
 * 
 * @param ip The IP 
 */
IP::IP(uint32_t ip) {
    this->ip = ip;
    formatIP();
}

/**
 * @brief Construct a new IP object using the string
 * 
 * @param ip The IP 
 */
IP::IP(const std::string& ip) {
    this->ipStr = ip;
    CompressIP();
}

/**
 * @brief Default construction. Will set IP to 0.0.0.0
 * 
 */
IP::IP() {

    this->ip = 0;
    this->formatIP();

}

/**
 * @brief Will convert a string to a uint32_t IP address
 * 
 * @param ip The IP in string format
 * @return uint32_t The IP in uint32_t format
 */
void IP::CompressIP() {
    
    this->ip = 0;

    /* Create a stringstream object */
    std::stringstream ss(this->ipStr);

    /* ipByte holds the byte of the IP to string in string format */
    std::string ipByte;

    /* Loop through the four bytes. For each byte, add it into the integer in reverse order. Shift zero for the first one, shift 8 for the next, ... */
    for (int i = (int)sizeof(uint32_t) - 1; i >= 0; i--) {
        std::getline(ss, ipByte, '.');
        ((std::byte*)&this->ip)[i] = (std::byte)std::stoi(ipByte);
        
    }

}

/**
 * @brief Internally decompresses IP into string
 * 
 */
void IP::formatIP() {
    
    for (int index = (int)sizeof(uint32_t) - 1; index >= 0; index--) {
        this->ipStr += std::to_string((int)((std::byte*)&this->ip)[index]);
        if (index != 0) {
            this->ipStr += ".";
        }
    }

}

/**
 * @brief Sets the IP address using the uint32_t format
 * 
 * @param ip The IP address in integer format
 */
void IP::setIP(uint32_t ip) {
    this->ip = ip;
    this->formatIP();
}

/**
 * @brief Sets the IP address using the string format
 * 
 * @param ip The IP address in string format
 */
void IP::setIP(const std::string& ip) {
    this->ipStr = ip;
    this->CompressIP();
}

/**
 * @brief Returns the IP in string format. '.' are used as delimiters
 * 
 * @return const std::string& The string format of the IP
 */
const std::string& IP::str() const {
    return this->ipStr;
}


/**
 * @brief Returns the IP in uint32_t format
 * 
 * @return uint32_t The IP address
 */
uint32_t IP::compressed() const {
    return this->ip;
}