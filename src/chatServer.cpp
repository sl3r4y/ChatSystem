#include "chatServer.h"

/**
 * The main and the one constructor of this class.
 * 
 * @param ipAddress The ip address of the server
 * @param port The port to used
 * @param maxOfConnection The max number of the connection in the same time
 * 
 * @throw SocketException
 */
ChatServer::ChatServer(std::string ipAddress, const unsigned short port, size_t maxOfConnection = 5) : ipAddress(ipAddress), port(port)
{
    // We create the socket which will wait for a new connection
    if ((this->socketFileDescriptor = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        throw SocketException("Error while the creation of the socket !");
    }

    // We fill the server description structure
    this->serverDescription.sin_family = AF_INET;
    this->serverDescription.sin_port = htons(port);
    this->serverDescription.sin_addr.s_addr = inet_addr(ipAddress.c_str());

    // We bind the socket with the server description structure
    if (bind(this->socketFileDescriptor, (sockaddr *)&(this->serverDescription), sizeof(sockaddr_in)) == -1)
    {
        throw SocketException("Can't bind the socket with the server description structure !");
    }

    this->nbConnection = 0;
    this->maxOfConnection = maxOfConnection;
    this->stopThreads = false;
}

/** The deconstructor of this class. */
ChatServer::~ChatServer()
{
    // We use the lock because we operate on shared variables
    this->theLock.lock();

    // We need to do this in case where one or more threads are blocked in read()
    // If there is no running threads then the socketFileDescriptor vector is empty
    for (auto fd : this->socketFileDescriptors)
    {
        int error = 0;
        socklen_t len = sizeof(int);
        int retval = getsockopt(fd, SOL_SOCKET, SO_ERROR, &error, &len);

        // If an error appears one the client socket before we close it then there will be an error
        // We compare the return value of getsockopt with 0 in order to not generate an error explain above
        if (retval == 0)
            if (close(fd) == -1)
                std::cerr << "Error while closing the client socket !" << std::endl;
    }

    // We delete all ruuning threads
    for (auto th : this->threads)
    {
        std::thread *t = std::get<0>(th);
        t->join();

        delete t;
    }

    this->theLock.unlock();

    // We close the file descriptor of the main socket
    if (close(this->socketFileDescriptor) == -1)
    {
        std::cerr << "Error while closing the main socket !" << std::endl;
    }
}

/** 
 * This method allows to put the server in listen mode in order to accept new connections.
 * 
 * @throw SocketException
 */
void ChatServer::run()
{
    socklen_t clientDescriptionLength{sizeof(sockaddr)};
    sockaddr_in clientDescription;
    int clientSocketFileDescriptor;

    // We set the socket in listen mode
    if (listen(this->socketFileDescriptor, this->maxOfConnection) == -1)
    {
        throw SocketException("Error while the setting the listen mode !");
    }

    std::cout << "IP address : " << this->ipAddress << std::endl << "Port : " << this->port << std::endl << "Waiting the first client..." << std::endl;

    // We create this thread in order to remove the memory allocated for threads which are terminated and to remove them of the threads vector.
    std::thread threadWhichRemoveOthersThreads([this]() {
        this->theLock.lock();

        auto it = this->threads.begin();
        while (it != this->threads.end())
        {

            if (std::get<2>(*it))
            {
                std::thread *t = std::get<0>(*it);
                t->join();
                delete t;
                it = this->threads.erase(it);
            }
            else
            {
                ++it;
            }
        }
        this->theLock.unlock();
    });

    // We create a loop in order to accept one connection and create one thread which will manage it.
    while (true)
    {
        // We accept a new connection and if there is an error then we throw an exception
        if ((clientSocketFileDescriptor = accept(this->socketFileDescriptor, (sockaddr *)&clientDescription, &clientDescriptionLength)) == -1)
        {
            this->stopThreads = true;
            throw SocketException("Can't accept a connection !");
        }

        std::cout << "A new client is connected (" << inet_ntoa(clientDescription.sin_addr) << ":" << clientDescription.sin_port << ", FD : " << clientSocketFileDescriptor << ")" << std::endl;

        this->theLock.lock();
        
        // We add the new socket file descriptor in the vector
        this->socketFileDescriptors.push_back(clientSocketFileDescriptor);

        this->nbConnection++;

        // We create a thread in order to get its messages and send them to others clients
        std::thread *t = new std::thread(&ChatServer::manageClient, this, clientSocketFileDescriptor, clientDescription);
        std::tuple<std::thread *, std::thread::id, bool> threadsTuple(std::make_tuple(t, t->get_id(), false));

        this->threads.push_back(threadsTuple);

        this->theLock.unlock();
    }
}

/** 
 * This method allows to get the pseudo of the client. 
 * 
 * @param socketFileDescriptor The file descriptor used for interact with the client
 * @param m Reference of message structure
 * 
 * @return false if an error appears or the stopThreads signal is triggered, true otherwise
 */
bool ChatServer::getPseudo(int socketFileDescriptor, Message &m)
{
    char pseudo[PSEUDO_LENGTH + 1];
    memset(pseudo, 0, PSEUDO_LENGTH + 1);

    if (!this->stopThreads)
    {
        // We receive the pseudo of the client.
        if (read(socketFileDescriptor, m.pseudo, PSEUDO_LENGTH) == -1)
        {
            std::cerr << "Error to receive the pseudo of client !" << std::endl;
            return false;
        }

        // We answer to the client by sending the received pseudo
        if (write(socketFileDescriptor, m.pseudo, strlen(pseudo)) == -1)
        {
            std::cerr << "Error while sending an answer to the client !" << std::endl;
            return false;
        }

        return true;
    }
    return false;
}

/**
 * This method allows to get one message send by the client.
 * 
 * @param socketFileDescriptor The file descriptor used for interact with the client
 * @param m Reference of message structure
 * 
 * @return false if an error appears or the stopThreads signal is triggered, true otherwise
 */
bool ChatServer::getMessage(int socketFileDescriptor, Message &m)
{
    int retval;

    if (this->stopThreads)
        return false;

    if (memset(m.message, 0, MESSAGE_SIZE + 1) == nullptr)
        std::cerr << "Error memset()" << std::endl;

    // We wait the message of the client
    if ((retval = read(socketFileDescriptor, m.message, MESSAGE_SIZE)) <= 0)
    {
        if (retval == -1 && errno != ECONNRESET && errno != ECONNREFUSED && errno != ECONNABORTED)
            std::cerr << "Error while receiving the message from the client (" << m.pseudo << ", FD : " << socketFileDescriptor << ") !" << std::endl;
            
        return false;
    }

    this->theLock.lock();
    std::cout << m.pseudo << " : " << m.message << std::endl;

    // We send the message to others clients
    for (auto fd : this->socketFileDescriptors)
        if (fd != socketFileDescriptor)
            if (write(fd, &m, sizeof(m)) == -1)
                std::cerr << "Impossible to send the message to one client !" << std::endl;

    this->theLock.unlock();
    
    return true;
}

/**
 * This method will used by the threads in order to interact with clients.
 * 
 * @param socketFileDescriptor This is the file descriptor of the socket
 * @param clientDescriptor This is the client description structure
 */
void ChatServer::manageClient(int socketFileDescriptor, struct sockaddr_in clientDescription)
{
    Message m;
    bool noError{false};

    if (!this->stopThreads && this->getPseudo(socketFileDescriptor, m))
    {
        do
        {
            noError = this->getMessage(socketFileDescriptor, m);
        } while (!this->stopThreads && noError);
    }

    std::cout << "End of the connection with " << m.pseudo << std::endl;

    if (close(socketFileDescriptor) == -1)
        std::cerr << "Error while closing the socket (" << inet_ntoa(clientDescription.sin_addr) << ":" << clientDescription.sin_port << ")" << std::endl;

    // We signal the main thread that this thread is terminated
    this->theLock.lock();

    for (std::tuple<std::thread *, std::thread::id, bool> tuple : this->threads)
        if (std::get<1>(tuple) == std::this_thread::get_id())
            std::get<2>(tuple) = true;

    this->socketFileDescriptors.erase(std::remove(this->socketFileDescriptors.begin(), this->socketFileDescriptors.end(), socketFileDescriptor), this->socketFileDescriptors.end());
    this->theLock.unlock();
}

/**
 * This method allows to get the ip address.
 * 
 * @return A string containing the ip address
 */
std::string ChatServer::getIpAddress() const noexcept
{
    return this->ipAddress;
}

/**
 * This method allows to get the port.
 * 
 * @return The port of the server
 */
unsigned short ChatServer::getPort() const noexcept
{
    return this->port;
}
