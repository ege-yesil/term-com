#ifndef COMMANDS_H
#define COMMANDS_H
#include <string.h>
#include <stdio.h>

#define BUF_SIZE 1024

enum commandType {
    SAY,
    KICK, 
    KICKALL
};

// buf is the string buffer data is written into
// id is the communication buffer id managed by the commander thread
// each time a buffer is sent its incremented by one
// it is used for the manager thread to process the buffer
//
// cnts is the length of the buffer buf
// type specifies the type of the command
struct shmpBuf {
    char buf[BUF_SIZE];
    volatile size_t id;
    size_t cnts;
    enum commandType type;
};

char* comTypeToString(enum commandType type);

// Exec functions or command functions take a shared memory space and execute the said command.
// The shared memory space needs to be readable and writable
// The shared memory space needs to be the one used by the commander thread to communicate with the manager threads. 
void exec_say(struct shmpBuf* shmp, char* str);
void exec_kick(struct shmpBuf* shmp, char* who,char* str);
void exec_kickAll(struct shmpBuf* shmp, char* str);

#endif
