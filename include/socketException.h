#ifndef SOCKET_EXCEPTION_H_
#define SOCKET_EXCEPTION_H_

#include <exception>
#include <string>

/**
 * @author sleray
 * @version 0.0.1
 * 
 * This is an derived class of exception class.
 */
class SocketException : public std::exception
{
public:
    SocketException(const char* message) : message(message) {}
    const char* what() { return this->message.c_str(); }
private:
    std::string message;
};

#endif