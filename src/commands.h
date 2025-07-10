#ifndef COMMANDS_H
#define COMMANDS_H
#include <string.h>
#include <stdio.h>

#define BUF_SIZE 1024
#define MAX_CMD_SIZE 128  // this just effects the global command history
                          // after it fills up the program starts again from 0

enum commandType {
    NOCMD,
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
struct command {
    char buf[BUF_SIZE];
    size_t cnts;
    enum commandType type;
};

struct commandShm {
    struct command commands[MAX_CMD_SIZE];
    volatile size_t newestCommand;
};

void resetCommandShm(struct commandShm* shmp);

char* cmdTypeToString(enum commandType type);

// Exec functions or command functions take a shared memory space and execute the said command.
// The shared memory space needs to be readable and writable
// The shared memory space needs to be the one used by the commander thread to communicate with the manager threads. 
void exec_say(struct commandShm* cmd, char* str);
void exec_kick(struct commandShm* cmd, char* who,char* str);
void exec_kickAll(struct commandShm* cmd, char* str);

#endif
