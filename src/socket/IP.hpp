/**
 * @file IP.hpp
 * @author Tyler Johnson (tjohn73@uwo.ca)
 * @brief This file will hold the IP address and handle conversions
 * @version 0.1
 * @date 2023-03-30
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#pragma once

#include <string>
#include <sstream>
#include <stdint.h>

/**
 * @brief This class will hold the IP address and handle conversions
 * 
 */
class IP {

private:

    /**
     * @brief The IP address in uint32_t format
     * 
     */
    uint32_t ip;

    /**
     * @brief The IP address in string format
     * 
     */
    std::string ipStr;

    /**
     * @brief Internally compresses IP into bytes
     * 
     */
    void CompressIP();

    /**
     * @brief Internally decompresses IP into string
     * 
     */
    void formatIP();

public:

    /**
     * @brief Construct a new IP object using the uint32_t
     * 
     * @param ip The IP 
     */
    IP(uint32_t ip);

    /**
     * @brief Construct a new IP object using the string
     * 
     * @param ip String format of the IP address
     */
    IP(const std::string& ip);

    /**
     * @brief Default construction. Will set IP to 0.0.0.0
     * 
     */
    IP();
  	
  	/**
     * @brief Default destructor
     *
     */
  	~IP() = default;

    /**
     * @brief Returns the IP in string format. '.' are used as delimiters
     * 
     * @return const std::string& The string format of the IP
     */
    const std::string& str() const;

    /**
     * @brief Returns the IP in uint32_t format
     * 
     * @return uint32_t The IP address
     */
    uint32_t compressed() const;

    /**
     * @brief Sets the IP address using the uint32_t format
     * 
     * @param ip The IP address in integer format
     */
    void setIP(uint32_t ip);

    /**
     * @brief Sets the IP address using the string format
     * 
     * @param ip The IP address in string format
     */
    void setIP(const std::string& ip);



};
