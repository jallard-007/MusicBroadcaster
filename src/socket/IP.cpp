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

IP::IP(uint32_t ip) {
    this->ip = ip;
    formatIP();
}

IP::IP(const std::string& ip) {
    this->ipStr = ip;
    CompressIP();
}

IP::IP() {

    this->ip = 0;
    this->formatIP();

}

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

void IP::formatIP() {
    
    for (int index = (int)sizeof(uint32_t) - 1; index >= 0; index--) {
        this->ipStr += std::to_string((int)((std::byte*)&this->ip)[index]);
        if (index != 0) {
            this->ipStr += ".";
        }
    }

}

void IP::setIP(uint32_t ip) {
    this->ip = ip;
    this->formatIP();
}

void IP::setIP(const std::string& ip) {
    this->ipStr = ip;
    this->CompressIP();
}

const std::string& IP::str() const {
    return this->ipStr;
}

uint32_t IP::compressed() const {
    return this->ip;
}
