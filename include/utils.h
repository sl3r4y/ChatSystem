#ifndef MESSAGE_H
#define MESSAGE_H

const int PSEUDO_LENGTH{31};
const int MESSAGE_SIZE{255};

typedef struct Message
{
    char pseudo[32];
    char message[256];
} Message;
#endif