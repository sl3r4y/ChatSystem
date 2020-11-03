#ifndef TCHAT_SERVER_H_
#define TCHAT_SERVER_H_

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <iostream>
#include <vector>
#include <thread>
#include <mutex>
#include <string>
#include <string.h>
#include <tuple>
#include <algorithm>
#include <cerrno> 

#include "socketException.h"
#include "utils.h"

/**
 * @author sleray
 * @version 0.0.1
 * 
 * This class allows to create a tchat server. 
 */
class ChatServer
{
public:
    /** The main and the one constructor of this class. */
    ChatServer(std::string ipAddress, const unsigned short port, size_t maxOfConnection);

    /** The deconstructor of this class. */
    ~ChatServer();

    /** This method allows to put the server in listen mode in order to accept new connections. */
    void run();

    /** This method will used by the threads in order to interact with clients. */
    void manageClient(int socketFileDescriptor, struct sockaddr_in clientDescription);

    /** This method allows to get the ip address. */
    std::string getIpAddress() const noexcept;

    /** This method allows to get the port. */
    unsigned short getPort() const noexcept;

    /** This method allows to get the pseudo of the client. */
    bool getPseudo(int socketFileDescriptor, Message &m);

    /** This method allows to get one message send by the client. */
    bool getMessage(int socketFileDescriptor, Message &m);
private:
    /** This is ip address of the server. */
    std::string ipAddress;

    /** This is the port of the server. */
    unsigned short port;

    /** This is the main socket file descriptor. */
    int socketFileDescriptor;

    /** This is the server description structure. */
    struct sockaddr_in serverDescription;

    /** This is the socket file descriptors list. Each file descriptor is a link to the connection with one client. */
    std::vector<int> socketFileDescriptors;

    /** This is the lock allows to make mutual exclusion between threads. */
    std::mutex theLock;

    /** This is the number of connections etablished. */
    size_t nbConnection;

    /** This is the max of connections accepted at a given moment. */
    size_t maxOfConnection;

    /** This is the list of threads used for manage the clients. */
    std::vector<std::tuple<std::thread*, std::thread::id, bool>> threads;

    /** This allows to stop the threads. */
    bool stopThreads;
};
#endif