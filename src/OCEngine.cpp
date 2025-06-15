#include <iostream>
#ifdef __linux__
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#endif

#include "Logger.hpp"
#include "OCEngine.hpp"

#include "TcpServer.hpp"
#include "TcpUtilities.hpp"

TcpServer server;
string incomingCharacterBuffer;
Logger *logger;

void onConnect(const int fd)
{
    *logger << INFO << "Connected on fd: " << fd << " IP: " << getIPbyFD(fd) << std::endl;
}

void onInput(const int fd, const char* incomingSocketData)
{

    string cmdLine;
    incomingCharacterBuffer.append(incomingSocketData);

    if (handleIncomingSocketDataAsLine(&cmdLine,&incomingCharacterBuffer) > 0) {
        *logger << INFO << "Received from fd: " << fd << " IP: " << getIPbyFD(fd) << std::endl;
        printHexlStringStream(cmdLine);

        *logger << INFO << "Sending to fd: " << fd << " IP: " << getIPbyFD(fd) << std::endl;
        TcpServer::sendMessage(fd, "checked - done\r\n");
        printHexlStringStream("checked - done\r\n");
    }
}

void onDisconnect(const int fd)
{
    *logger << INFO << "Disconnected on fd: " << fd << " IP: " << getIPbyFD(fd) << std::endl;
}


int main(int argc, char* argv[])
{
    /* Start */
    incomingCharacterBuffer.clear();

    logger = &Logger::getInstance(OC_LOG_FILE); // Create logger instance

    *logger << INFO << "OCEngine started" << endl;

    *logger << DEBUG << "argc: " << argc << endl;
    for (int i = 0; i < argc; ++i)    
        *logger << DEBUG << "argv[" << i << "]:" << argv[i] << endl;

    server.onConnected(&onConnect);
    server.onDisconnected(&onDisconnect);
    server.onReceivedData(&onInput);
    server.init();

    while (true)
    {
        server.loop();
    }
    
    *logger << INFO << "OCEngine stopped normaly" << endl;

    server.shutdown();

    return 0;
}
