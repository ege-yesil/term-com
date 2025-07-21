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

#include "serverModes.h"
#include "../util.h"
#include "../commands.h"

#define KICK_MESSAGE "You have been kicked from the server by the owner. Kick message: "

typedef void (*responseHandler)(int, const char*);
responseHandler handlers[] = {
    silience,
    echo
};

void changeServerResponseFn(size_t handlerIndex);

// Executes a command given by the commander thread
// Returns if the command was successful
bool executeCommand(int client, struct sockaddr_in addr, struct command cmd);

// This function will integrate any working server socket to the term-com pipeline
void listenToClients(int serverSocket);

// Starts a TCP server in the specified port with specified settings 
void startServer(int port, size_t maxCon);
#endif
