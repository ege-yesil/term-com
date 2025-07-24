#ifndef SERVER_H
#define SERVER_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/mman.h>
#include  <pthread.h>

#include "serverModes.h"
#include "../util.h"
#include "../commands.h"

#define KICK_MESSAGE "You have been kicked from the server by the owner. Kick message: "

struct clientData {                                                                                                           
    int client;
    struct sockaddr_in addr;
};

struct message {
    char* buf;
    size_t bufSize;
    int sender;
    int reciever; // -1 is to ther server
                  // -2 is to all clients
                  // any number other are interpreted as socket ids
    pthread_mutex_t lock;
};

struct serverState {
    size_t connected;
    size_t responseIndex;
    size_t nextConnection; // if a thread exits in the array will be an empty spot this is to decide the next clients thread id
    pthread_mutex_t lock;
};

// A structure that encapsulates all the things a thread might need
struct program {
    struct commandMem cmd;
    struct message* messages;
    struct serverState state;
};

typedef void (*responseHandler)(int, const char*);
responseHandler handlers[] = {
    silience,
    echo
};

struct program getProgram();
void setProgramSettings(struct program);

// Executes a command given by the commander thread
// Returns if the command was successful
bool executeCommand(struct clientData data, struct command cmd);

// This function will integrate any working server socket to the term-com pipeline
void listenToClients(int serverSocket, size_t maxCon);

// Starts a TCP server in the specified port with specified settings 
void startServer(int port, size_t maxPen, size_t maxCon);
#endif
