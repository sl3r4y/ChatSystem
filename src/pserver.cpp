#include "chatServer.h"

int main(int argc, const char *argv[])
{
    try
    {
        ChatServer s{std::string("0.0.0.0"), (unsigned short)54000, 5};

        s.run();
    }
    catch (SocketException &e)
    {
        std::cerr << e.what() << std::endl;
    }

    return 0;
}