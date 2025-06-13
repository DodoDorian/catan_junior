#include "TcpServer.hpp"
#include "OCEngine.hpp"
#include <string>
#include <cstring>
#include <arpa/inet.h> // For inet_ntop and inet_pton

/**
 * @brief Constructor for TcpServer. Initializes the logger and sets up the server with a default port.
 */
TcpServer::TcpServer() {
    logger = &Logger::getInstance(OC_LOG_FILE); // Create logger instance

#ifdef SERVER_DEBUG
    *logger << DEBUG << "[TcpServer] [CONSTRUCTOR] creating TCPServer with default port " << DEFAULT_PORT << std::endl;
#endif

#ifdef _WIN32
    // Initialize Winsock on Windows
    int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
      *logger << ERROR << "WSAStartup crashed with error: " << iResult << std::endl;
      exit(-1);
    }
#endif

    setup(DEFAULT_PORT); // Set up the server with the default port
}

/**
 * @brief Constructor for TcpServer with a specified port.
 *
 * @param port The port number to use for the server.
 */
TcpServer::TcpServer(int port) {
    logger = &Logger::getInstance(OC_LOG_FILE); // Create logger instance

#ifdef SERVER_DEBUG
    *logger << DEBUG << "[TcpServer] [CONSTRUCTOR] creating TCPServer with port " << port << " ..." << std::endl;
#endif

#ifdef _WIN32
    // Initialize Winsock on Windows
    int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
      *logger << ERROR << "WSAStartup crashed with error: " << iResult << std::endl;
        exit(-1);
    }
#endif

    setup(port); // Set up the server with the specified port
}

/**
 * @brief Destructor for TcpServer. Closes the server and cleans up resources.
 */
TcpServer::~TcpServer() {
#ifdef SERVER_DEBUG
    *logger << DEBUG << "[TcpServer] [DESTRUCTOR] closing server ..." << std::endl;
#endif

    closeConnection(mastersocket_fd); // Close the master socket connection

#ifdef _WIN32
    WSACleanup(); // Clean up Winsock on Windows
#endif
}

/**
 * @brief Shuts down the TcpServer by closing the connection.
 */
void TcpServer::shutdown() {
#ifdef SERVER_DEBUG
    *logger << DEBUG << "[TcpServer] [DEBUG] [SHUTDOWN] shutdown TcpServer ..." << std::endl;
#endif

    closeConnection(mastersocket_fd); // Close the master socket connection
}

/**
 * @brief Initializes the TcpServer by setting up the socket, binding, and starting to listen.
 */
void TcpServer::init() {
#ifdef SERVER_DEBUG
    *logger << DEBUG << "[TcpServer] [DEBUG] [INIT] initialize TcpServer ..." << std::endl;
#endif

    initializeSocket(); // Initialize the socket
    bindSocket();       // Bind the socket to the address
    startListen();      // Start listening for incoming connections
}

/**
 * @brief Main loop for the TcpServer, handling incoming connections and data.
 */
void TcpServer::loop() {
    tempfds = masterfds; // Copy master file descriptor set

#ifdef SERVER_DEBUG
    *logger << DEBUG << "[TcpServer] [LOOP] max fd = '" << maxfd << "'" << std::endl;
    *logger << DEBUG << "[TcpServer] [LOOP] calling select()" << std::endl;
#endif

    const int sel = select(static_cast<int>(maxfd) + 1, &tempfds, nullptr, nullptr, nullptr); // Wait for activity on sockets

#ifdef SERVER_DEBUG
    *logger << DEBUG << "[TcpServer] [LOOP] select() return value " << sel << ", evaluation ..." << std::endl;
#endif

    if (sel < 0) {
      *logger << ERROR << "[TcpServer] [LOOP] select() crashed with error: " << sel << std::endl;
        shutdown(); // Shutdown the server on error
    }

    for (SOCKET_DISCRIPTOR_ i = 0; i <= maxfd; i++) {
        if (FD_ISSET(i, &tempfds)) { // Check if socket is ready for reading
            if (mastersocket_fd == i) {
                handleNewConnection(); // Handle new incoming connection
            } else {
                recvInputFromExisting(i); // Receive input from an existing connection
            }
        }
    }
}

/**
 * @brief Sets the callback function to be called when a new connection is established.
 *
 * @param ncc Pointer to the callback function.
 */
void TcpServer::onConnected(void (*ncc)(SOCKET_DISCRIPTOR_ fd)) {
#ifdef SERVER_DEBUG
    *logger << DEBUG << "[TcpServer] [CALLBACK ONCONNECT] onConnect ..." << std::endl;
#endif

    connectedCallback = ncc; // Set the new connection callback
}

/**
 * @brief Sets the callback function to be called when data is received.
 *
 * @param rc Pointer to the callback function.
 */
void TcpServer::onReceivedData(void (*rc)(SOCKET_DISCRIPTOR_ fd, const char* buffer)) {
#ifdef SERVER_DEBUG
    *logger << DEBUG << "[TcpServer] [CALLBACK ONINPUT] onInput ..." << std::endl;
#endif

    receivedCallback = rc; // Set the received data callback
}

/**
 * @brief Sets the callback function to be called when a connection is disconnected.
 *
 * @param dc Pointer to the callback function.
 */
void TcpServer::onDisconnected(void (*dc)(SOCKET_DISCRIPTOR_ fd)) {
#ifdef SERVER_DEBUG
    *logger << DEBUG << "[TcpServer] [CALLBACK ONDISCONNECT] onDisconnect ..." << std::endl;
#endif

    disconnectedCallback = dc; // Set the disconnected callback
}

/**
 * @brief Sends a message to a specified socket descriptor.
 *
 * @param fd The socket descriptor to send the message to.
 * @param messageBuffer The message to send.
 * @return The number of bytes sent.
 */
int TcpServer::sendMessage(SOCKET_DISCRIPTOR_ fd, const char* messageBuffer) {
    return static_cast<int>(send(fd, messageBuffer, static_cast<int>(strlen(messageBuffer)), 0)); // Send the message
}

/**
 * @brief Overloaded function to send a message using a char pointer.
 *
 * @param fd The socket descriptor to send the message to.
 * @param messageBuffer The message to send.
 * @return The number of bytes sent.
 */
int TcpServer::sendMessage(SOCKET_DISCRIPTOR_ fd, char* messageBuffer) {
    return static_cast<int>(send(fd, messageBuffer, static_cast<int>(strlen(messageBuffer)), 0)); // Send the message
}

/**
 * @brief Sets up the server socket with the specified port.
 *
 * @param port The port number to use for the server.
 */
void TcpServer::setup(int port) {
#ifdef SERVER_DEBUG
    *logger << DEBUG << "[TcpServer] [SETUP] creating AF_INET6 STREAM socket with port " << port << std::endl;
#endif

    mastersocket_fd = socket(AF_INET6, SOCK_STREAM, 0); // Create an IPv6 socket
    if (mastersocket_fd < 0) {
        *logger << ERROR << "[TcpServer] [SETUP] socket() crashed with error: " << mastersocket_fd << std::endl;

#ifdef _WIN32
        WSACleanup(); // Clean up Winsock on Windows
#endif

        exit(-1); // Exit on socket creation failure
    }

    FD_ZERO(&masterfds); // Initialize the master file descriptor set
    FD_ZERO(&tempfds);   // Initialize the temporary file descriptor set
    memset(&servaddr, 0, sizeof(servaddr)); // Clear the server address structure

    auto* addr6 = reinterpret_cast<struct sockaddr_in6*>(&servaddr);
    addr6->sin6_family = AF_INET6; // Set address family to IPv6
    addr6->sin6_addr = in6addr_any; // Accept connections from any IPv6 address
    addr6->sin6_port = htons(port); // Set the port number

    memset(input_buffer, 0, INPUT_BUFFER_SIZE); // Clear the input buffer
}

/**
 * @brief Initializes the socket options.
 */
void TcpServer::initializeSocket() {
#ifdef SERVER_DEBUG
    *logger << DEBUG << "[TcpServer] socket initialization" << std::endl;
#endif

    int opt_value = 1;
    int ret_test = setsockopt(mastersocket_fd, SOL_SOCKET, SO_REUSEADDR, (char*)&opt_value, sizeof(int)); // Set socket options

    if (ret_test < 0) {
        *logger << ERROR << "[TcpServer] [ERROR] setsockopt() crashed with error:" << ret_test << std::endl;
        shutdown(); // Shutdown on error
    } else {
        #ifdef SERVER_DEBUG
            *logger << DEBUG << "[TcpServer] setsockopt(): " << ret_test << std::endl;
        #endif
    }
}

/**
 * @brief Binds the socket to the specified address and port.
 */
void TcpServer::bindSocket() {
#ifdef SERVER_DEBUG
    *logger << DEBUG << "[TcpServer] bind ..." << std::endl;
#endif

    const int bind_ret = bind(mastersocket_fd, reinterpret_cast<struct sockaddr*>(&servaddr), sizeof(servaddr)); // Bind the socket

    if (bind_ret < 0) {
        *logger << ERROR << "[TcpServer] [ERROR] bind() crashed with error: " << std::endl;
    } else {
        #ifdef SERVER_DEBUG
            *logger << DEBUG << "[TcpServer] bind(): " << bind_ret << std::endl;
        #endif
    }

    FD_SET(mastersocket_fd, &masterfds); // Add the master socket descriptor to the master FD set
    maxfd = mastersocket_fd; // Set the current maximum file descriptor
}

/**
 * @brief Starts listening for incoming connections on the socket.
 */
void TcpServer::startListen() const {
#ifdef SERVER_DEBUG
    *logger << DEBUG << "[TcpServer] start listen ..." << std::endl;
#endif

    int listen_ret = listen(mastersocket_fd, 3); // Start listening with a backlog of 3

    if (listen_ret < 0) {
        *logger << ERROR << "[TcpServer] [ERROR] listen() crashed with error: " << listen_ret << std::endl;
    } else {
        #ifdef SERVER_DEBUG
            *logger << DEBUG << "[TcpServer] listen(): " << listen_ret << std::endl;
        #endif
    }
}

/**
 * @brief Handles a new incoming connection.
 */
void TcpServer::handleNewConnection() {
#ifdef SERVER_DEBUG
    *logger << DEBUG << "[TcpServer] [CONNECTION] handle a new connection" << std::endl;
#endif

    socklen_t addrlen = sizeof(client_addr);
    tempsocket_fd = accept(mastersocket_fd, reinterpret_cast<struct sockaddr*>(&client_addr), &addrlen); // Accept a new connection

    if (tempsocket_fd < 0) {
        *logger << ERROR << "[TcpServer] [ERROR] accept() crashed with error: " << tempsocket_fd << std::endl;
        exit(-1); // Exit on accept failure
    } else {
        FD_SET(tempsocket_fd, &masterfds); // Add the new socket to the master FD set

        if (tempsocket_fd > maxfd) {
            maxfd = tempsocket_fd; // Update the maximum file descriptor

#ifdef SERVER_DEBUG
            *logger << DEBUG << "[TcpServer] max file descriptor increment to " << maxfd << std::endl;
#endif
        }

#ifdef SERVER_DEBUG
        *logger << DEBUG << "[TcpServer] [CONNECTION] new connection on file descriptor: " << tempsocket_fd << std::endl;
#endif
    }
    connectedCallback(tempsocket_fd); // Call the connected callback
}

/**
 * @brief Receives input from an existing connection.
 *
 * @param fd The socket descriptor to receive data from.
 */
void TcpServer::recvInputFromExisting(SOCKET_DISCRIPTOR_ fd) {
    int nbrecv = static_cast<int>(recv(fd, input_buffer, INPUT_BUFFER_SIZE, 0)); // Receive data from the socket
    if (nbrecv <= 0) {
        if (nbrecv == 0) {
            disconnectedCallback(fd); // Call the disconnected callback
            closeConnection(fd); // Close the connection
            FD_CLR(fd, &masterfds); // Remove client FD from the FD set
            return;
        } else {
            *logger << ERROR << "[TcpServer] [ERROR] recv() called with errors" << std::endl;
        }
        closeConnection(fd); // Close the connection on error
        FD_CLR(fd, &masterfds); // Remove client FD from the FD set
        return;
    }

#ifdef SERVER_DEBUG
    *logger << DEBUG << "[TcpServer] [RECV] receive data from client" << std::endl;
#endif

    receivedCallback(fd, input_buffer); // Call the received data callback
    memset(input_buffer, 0, INPUT_BUFFER_SIZE); // Reset the input buffer
}

/**
 * @brief Closes the connection for a specified socket descriptor.
 *
 * @param fd The socket descriptor to close.
 * @return The result of the close operation.
 */
int TcpServer::closeConnection(SOCKET_DISCRIPTOR_ fd) {
    FD_CLR(fd, &masterfds); // Remove the socket from the master FD set
#if defined(__linux__) || defined(__APPLE__)
    return close(fd); // Close the socket on Linux or macOS
#elif _WIN32
    return closesocket(fd); // Close the socket on Windows
#endif
}