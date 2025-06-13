#ifndef TCP_SERVER_  // Include guard to prevent multiple inclusions of this header file
#define TCP_SERVER_

#include <iostream>
#include "TcpUtilities.hpp" // Include custom TCP utility functions
#include "Logger.hpp" // Include logger for logging messages

// Platform-specific includes and definitions
#if defined (__linux__) || defined (__APPLE__)
    #include <unistd.h> // For POSIX operating system API
    #include <sys/select.h> // For select function
    #include <sys/types.h> // For data types used in system calls
    #include <netinet/in.h> // For internet address family
    #include <sys/socket.h> // For socket functions
    #define SOCKET_DISCRIPTOR_ int // Define SOCKET_DISCRIPTOR_ as int for socket descriptors
#elif _WIN32
    #include <cstdint> // For fixed-width integer types
    #include <WinSock2.h> // For Windows Sockets API
    #include <WS2tcpip.h> // For Windows TCP/IP functions
    #define SOCKET_DISCRIPTOR_ SOCKET // Define SOCKET_DISCRIPTOR_ as SOCKET for Windows
#endif

#define INPUT_BUFFER_SIZE 1024 // Size of the input buffer
#define DEFAULT_PORT 38233 // Default port for the server

#define SERVER_DEBUG true // Enable server debugging

/**
 * @class TcpServer
 * @brief A class to handle TCP server operations such as accepting connections and receiving data.
 */
class TcpServer {
public:
    /**
     * @brief Default constructor for TcpServer.
     */
    TcpServer();

    /**
     * @brief Constructor for TcpServer with a specified port.
     *
     * @param port The port number to use for the server.
     */
    explicit TcpServer(int port);

    /**
     * @brief Destructor for TcpServer.
     */
    virtual ~TcpServer();

#ifdef _WIN32
    WSADATA wsaData; // Structure to hold Windows Sockets data
#endif

    /**
     * @brief Shuts down the server and closes connections.
     */
    void shutdown();

    /**
     * @brief Initializes the server by setting up sockets and binding.
     */
    void init();

    /**
     * @brief Main loop for the server to handle incoming connections and data.
     */
    void loop();

    /**
     * @brief Sets the callback function to be called when a new connection is established.
     *
     * @param ncc Pointer to the callback function.
     */
    void onConnected(void (*ncc)(SOCKET_DISCRIPTOR_ fd));

    /**
     * @brief Sets the callback function to be called when data is received.
     *
     * @param rc Pointer to the callback function.
     */
    void onReceivedData(void (*rc)(SOCKET_DISCRIPTOR_ fd, const char* buffer));

    /**
     * @brief Sets the callback function to be called when a connection is disconnected.
     *
     * @param dc Pointer to the callback function.
     */
    void onDisconnected(void (*dc)(SOCKET_DISCRIPTOR_ fd));

    /**
     * @brief Sends a message to a specified socket descriptor.
     *
     * @param fd The socket descriptor to send the message to.
     * @param messageBuffer The message to send.
     * @return The number of bytes sent.
     */
    static int sendMessage(SOCKET_DISCRIPTOR_ fd, const char* messageBuffer) ;

    /**
     * @brief Overloaded function to send a message using a char pointer.
     *
     * @param fd The socket descriptor to send the message to.
     * @param messageBuffer The message to send.
     * @return The number of bytes sent.
     */
    static int sendMessage(SOCKET_DISCRIPTOR_ fd, char* messageBuffer) ;

    /**
     * @brief Closes the connection for a specified socket descriptor.
     *
     * @param fd The socket descriptor to close.
     * @return The result of the close operation.
     */
    int closeConnection(SOCKET_DISCRIPTOR_ fd);

private:
    Logger *logger; // Pointer to the logger instance

    fd_set masterfds{}; // Master file descriptor set
    fd_set tempfds{}; // Temporary file descriptor set

    SOCKET_DISCRIPTOR_ maxfd{}; // Maximum file descriptor

    SOCKET_DISCRIPTOR_ mastersocket_fd{}; // Master socket file descriptor
    SOCKET_DISCRIPTOR_ tempsocket_fd{}; // Temporary socket file descriptor

    struct sockaddr_storage client_addr{}; // Structure to hold client address
    struct sockaddr_storage servaddr{}; // Structure to hold server address

    char input_buffer[INPUT_BUFFER_SIZE]{}; // Buffer for incoming data
    char remote_ip[INET6_ADDRSTRLEN]{}; // Buffer for remote IP address

    void (*connectedCallback) (SOCKET_DISCRIPTOR_ fd){}; // Callback for new connections
    void (*receivedCallback) (SOCKET_DISCRIPTOR_ fd, const char* buffer){}; // Callback for received data
    void (*disconnectedCallback) (SOCKET_DISCRIPTOR_ fd){}; // Callback for disconnections

    /**
     * @brief Sets up the server socket with the specified port.
     *
     * @param port The port number to use for the server.
     */
    void setup(int port);

    /**
     * @brief Initializes the socket options.
     */
    void initializeSocket();

    /**
     * @brief Binds the socket to the specified address and port.
     */
    void bindSocket();

    /**
     * @brief Starts listening for incoming connections on the socket.
     */
    void startListen() const;

    /**
     * @brief Handles a new incoming connection.
     */
    void handleNewConnection();

    /**
     * @brief Receives input from an existing connection.
     *
     * @param fd The socket descriptor to receive data from.
     */
    void recvInputFromExisting(SOCKET_DISCRIPTOR_ fd);
};

#endif