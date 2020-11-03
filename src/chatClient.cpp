#include "chatClient.h"

/**
 * The main and the one constructor of this class.
 * 
 * @param ipAddress This is the ip address of the server
 * @param port This is the port of the server
 * @param pseudo This is the pseudo used
 * 
 * @throw SocketException 
 */
ChatClient::ChatClient(std::string ipAddress, unsigned short port, std::string pseudo) : ipAddress(ipAddress), port(port), pseudo(pseudo)
{
    // We create a socket
    if ((this->socketFileDescriptor = socket(AF_INET, SOCK_STREAM, 0)) == -1)
        throw SocketException("Can't create a socket !");

    // We fill the server description structure
    this->serverDescription.sin_family = AF_INET;
    this->serverDescription.sin_port = htons(port);
    this->serverDescription.sin_addr.s_addr = inet_addr(ipAddress.c_str());
}

/** 
 * The deconstructor of this class.
 */
ChatClient::~ChatClient()
{
    // We close the file descriptor of the main socket
    if (close(this->socketFileDescriptor) == -1)
    {
        std::cerr << "Error while closing the main socket !" << std::endl;
    }
}

/**
 * This method allows to connect to the server.
 */
void ChatClient::connectToServer()
{
    bool timeoutTriggered{false};
    bool answerIsReceived{false};
    bool error{false};
    Message m;

    // We connect us to the server
    if (connect(this->socketFileDescriptor, (sockaddr *)&(this->serverDescription), sizeof(sockaddr)) == -1)
        throw SocketException("Can't connect to the server !");

    std::cout << "Connection established with the server !" << std::endl;

    // We create a thread which wait to receive an answer
    std::thread threadReader([&answerIsReceived, &error](int socketFileDescriptor, const char *pseudo) {
        ssize_t nb;
        char buffer[PSEUDO_LENGTH + 1];

        if ((nb = read(socketFileDescriptor, buffer, PSEUDO_LENGTH)) <= 0)
            error = true;

        if (strncmp(buffer, pseudo, PSEUDO_LENGTH) == 0)
            answerIsReceived = true;
        else
            error = false;
    }, this->socketFileDescriptor, this->pseudo.c_str());

    // We create a timeout in order to close the connection if the server don't send the pseudo like answer
    std::thread threadTimeout([&timeoutTriggered]() { std::this_thread::sleep_for(std::chrono::milliseconds(5000)); timeoutTriggered = true; });

    // We send the pseaudo to the server
    do
    {
        if (write(this->socketFileDescriptor, this->pseudo.c_str(), this->pseudo.size()) == -1)
            std::cerr << "Can't send the pseudo to the server !" << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(500));

    } while (!timeoutTriggered && !error && !answerIsReceived);

    // We detach the threads in order to not wait that thread finish in case of an error appears
    threadReader.detach();
    threadTimeout.detach();

    if ((timeoutTriggered && !answerIsReceived) || error)
        throw SocketException("Impossible to send the pseudo to the server !");

    std::cout << "All be alright !" << std::endl;

    // This thread will update the screen with new messages
    std::thread threadMessages(&ChatClient::getMessagesFromTheServer, this);

    // The main thread will get message enter by the user and send it to the server
    while (true)
    {
        memset(m.message, 0, MESSAGE_SIZE);
 
        std::cin.getline(m.message, MESSAGE_SIZE);

        if (write(this->socketFileDescriptor, &m.message, strlen(m.message)) == -1)
        {
            std::cerr << "Impossible to send the message to the server !" << std::endl;
            break;
        }
    }
    
    threadMessages.join();

    std::cout << "End connection with the server !" << std::endl;
}

/**
 * This method allows to receive the messages sended to the server by others clients and show them to the screen. 
 */
void ChatClient::getMessagesFromTheServer()
{
    int retval;
    Message m;

    while (true)
    {
        memset(m.pseudo, 0, PSEUDO_LENGTH + 1);
        memset(m.message, 0, MESSAGE_SIZE + 1);
        if ((retval = read(this->socketFileDescriptor, &m, sizeof(m))) == -1)
            std::cerr << "Error while receiving a message from the server !" << std::endl;

        this->theLock.lock();
        
        std::cout << m.pseudo << " : " << m.message << std::endl;

        this->theLock.unlock();
    }
}