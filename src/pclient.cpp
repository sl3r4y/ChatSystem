#include "chatClient.h"

int main(int argc, const char *argv[])
{
    if (argc < 2)
    {
        std::cerr << "The pseudo is missing !" << std::endl;
        return EXIT_FAILURE;
    }
    try
    {
        ChatClient c{std::string("127.0.0.1"), (unsigned short)54000, std::string(argv[1])};
        c.connectToServer();
    }
    catch (SocketException &e)
    {
        std::cerr << e.what() << std::endl;
    }

    return 0;
}