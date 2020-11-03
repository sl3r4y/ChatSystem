#ifndef CHAT_CLIENT_H_
#define CHAT_CLIENT_H_

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <iostream>
#include <sstream>
#include <string>
#include <string.h>
#include <thread>
#include <chrono>
#include <mutex>

#include "socketException.h"
#include "utils.h"

/**
 * @author sleray
 * @version 0.0.1
 * 
 * This class allows to create a tchat client.
 */
class ChatClient
{
public:
    /** The main and the one constructor of this class. */
    ChatClient(std::string ipAddress, unsigned short port, std::string pseudo);

    /** The deconstructor of this class. */
    ~ChatClient();

    /** This method allows to connect to the server. */
    void connectToServer();

    /** This method allows to receive the messages sended to the server by others clients and show them to the screen. */
    void getMessagesFromTheServer();
private:
    /** This is the ip address of the server. */
    std::string ipAddress;

    /** This is the port of the server. */
    unsigned short port;

    /** This is the pseudo used. */
    std::string pseudo;

    /** This is the file descriptor of the socket. */
    int socketFileDescriptor;

    /** This is the server description structure. */
    struct sockaddr_in serverDescription;

    /** This is the lock allows to make mutual exclusion between threads. */
    std::mutex theLock;
};
#endif