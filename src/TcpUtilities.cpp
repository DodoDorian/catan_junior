#include "TcpUtilities.hpp"
#include "Logger.hpp"
#include <iostream> // Include for input/output stream operations
#include <sys/socket.h> // Include for socket-related functions and definitions
#include <netinet/in.h> // Include for internet address family
#include <cstring>      // Include for string manipulation functions
#include <arpa/inet.h>  // Include for inet_ntop and inet_pton functions
#include <regex>

/**
 * @brief Prints the hexadecimal representation of a given string buffer.
 *
 * This function formats the buffer into a hexadecimal string and prints it to the logger.
 * It also prints the ASCII representation of the data alongside the hex values.
 *
 * @param buffer The string buffer to be printed in hex format.
 */
void printHexlStringStream(const std::string& buffer)
{
    Logger& logger = Logger::getInstance(); // Get the logger instance
    std::ostringstream hexStream; // Stream to hold the hex representation

    string text; // String to hold printable characters

    for (unsigned int idx = 0; idx < buffer.length(); ++idx)
    {
        // Start a new line every 16 bytes
        if (idx % 16 == 0)
        {
            if (idx != 0)
            {
                logger << text << std::endl; // Log the printable text from the previous line
            }
            logger << "DATA "; // Log the data label
            text = ""; // Reset the text for the new line
        }

        // Convert byte to hex and append to hexStream
        hexStream << std::setw(2) << std::setfill('0') << std::hex
                  << static_cast<int>(static_cast<unsigned char>(buffer[idx])) << ":"; // Append hex value

        // Check if the character is printable
        if (isprint(static_cast<unsigned char>(buffer[idx])))
        {
            text += buffer[idx]; // Append printable character to text
        } else
        {
            text += "."; // Append a dot for non-printable characters
        }

        // Log the hex representation at the end of each line or at the end of the buffer
        if (idx % 16 == 15 || idx == buffer.length() - 1)
        {
            logger << hexStream.str(); // Log the hex string
            hexStream.str(""); // Clear hexStream
            hexStream.clear(); // Clear any error flags
        }
    }

    // Fill in the remaining space with "00:" and dots for unfilled lines
    for (unsigned int i = buffer.length(); i % 16 != 0 && i < buffer.length() + 16; ++i)
    {
        logger << "00" << ":"; // Log "00:" for unfilled bytes
        text += "."; // Add a dot for unfilled bytes
    }

    logger << text << std::endl; // Log the remaining printable text
}

/**
 * @brief Retrieves the IP address of a client connected to a socket.
 *
 * This function uses the socket file descriptor to get the client's address and
 * converts it to a string format.
 *
 * @param fd The socket file descriptor of the client.
 * @return A pointer to a string containing the IP address, or nullptr on failure.
 */
char* getIPbyFD(const int fd)
{
    sockaddr_storage clientAddr{}; // Structure to hold client address
    socklen_t addrLen = sizeof(clientAddr); // Size of the address structure

    // Get the peer name (address) of the connected socket
    if (getpeername(fd, reinterpret_cast<struct sockaddr*>(&clientAddr), &addrLen) == -1)
    {
        std::cerr << "error while getting the client address: " << strerror(errno) << std::endl; // Log error
        return nullptr; // Return nullptr on error
    }

    char* ipStr = new char[INET6_ADDRSTRLEN]; // Allocate space for the IP address string

    // Check if the address is IPv4
    if (clientAddr.ss_family == AF_INET)
    {
        const auto* addr4 = reinterpret_cast<sockaddr_in*>(&clientAddr); // Cast to IPv4 address structure
        // Convert the IPv4 address to string format
        if (inet_ntop(AF_INET, &addr4->sin_addr, ipStr, INET6_ADDRSTRLEN) == nullptr)
        {
            std::cerr << "error while converting an IPv4 address: " << strerror(errno) << std::endl; // Log error
            delete[] ipStr; // Free allocated memory
            return nullptr; // Return nullptr on error
        }
    }
    // Check if the address is IPv6
    else if (clientAddr.ss_family == AF_INET6)
    {
        const sockaddr_in6* addr6 = reinterpret_cast<sockaddr_in6*>(&clientAddr); // Cast to IPv6 address structure
        // Convert the IPv6 address to string format
        if (inet_ntop(AF_INET6, &addr6->sin6_addr, ipStr, INET6_ADDRSTRLEN) == nullptr)
        {
            std::cerr << "error while converting an IPv6 address: " << strerror(errno) << std::endl; // Log error
            delete[] ipStr; // Free allocated memory
            return nullptr; // Return nullptr on error
        }
    }
    else
    {
        std::cerr << "unknown address type" << std::endl; // Log unknown address type
        delete[] ipStr; // Free allocated memory
        return nullptr; // Return nullptr for unknown address types
    }

    return ipStr; // Return the IP address string
}

int handleIncomingSocketDataAsLine(std::string *line, std::string *inSockStr) {

    // Replace carriage return with newline and remove consecutive newlines
    *inSockStr = std::regex_replace(*inSockStr, std::regex("\r"), "\n");
    *inSockStr = std::regex_replace(*inSockStr, std::regex("\n{2,}"), "\n"); // Use {2,} to replace multiple newlines

    // Check if the last character is a newline
    if (!inSockStr->empty() && inSockStr->back() == '\n') {
        // Count the number of lines
        const int countLine = std::count(inSockStr->begin(), inSockStr->end(), '\n');

        // If there are lines to process
        if (countLine > 0) {
            std::stringstream inStreamSockStr(*inSockStr);
            for (int index = 0; index < countLine; index++) {
                std::getline(inStreamSockStr, *line);
                if (index == countLine - 1) {
                    // Keep remaining data in inSockStr after the last line
                    *inSockStr = inStreamSockStr.str().substr(inStreamSockStr.tellg());
                }
                return 1; // Return after processing each line
            }
        }
        inSockStr->clear(); // Clear input string after processing
    }
    return 0; // No complete line processed
}