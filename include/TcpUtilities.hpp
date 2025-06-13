#ifndef TCP_UTILITIES_HPP  // Include guard to prevent multiple inclusions of this header file
#define TCP_UTILITIES_HPP

#include <string>   // Include for using std::string
#include <iomanip>  // Include for input/output manipulators

using namespace std;

/**
 * @brief Prints the hexadecimal representation of a given string buffer.
 *
 * This function formats the buffer into a hexadecimal string and prints it to the standard output.
 *
 * @param buffer The string buffer to be printed in hex format.
 */
void printHexlStringStream(const string &buffer);

/**
 * @brief Retrieves the IP address of a client connected to a socket.
 *
 * This function uses the socket file descriptor to get the client's address and
 * converts it to a string format.
 *
 * @param fd The socket file descriptor of the client.
 * @return A pointer to a string containing the IP address, or nullptr on failure.
 */
char* getIPbyFD(int fd);

int handleIncomingSocketDataAsLine(std::string *line, std::string *inSockStr);
#endif