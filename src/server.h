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

#include "util.h"
#include "commands.h"

#define KICK_MESSAGE "You have been kicked from the server by the owner. Kick message: " 

// Executes a command given by the commander thread
// Returns if the command was successful
bool executeCommand(int client, struct sockaddr_in addr, char* shmpPath, struct shmpBuf* shmp);

// Reads and parses any input from the stdin
// Returns a char* array of size parameter segSize
char** parseCommand(size_t segSize);

// This function will integrate any working server socket to the term-com pipeline
// c2mShmpName is the file name which is going to be used for the commander to communicate with manager threads
// If the server is started via startServer c2mShmpName will be "/commanderToManager"
void listenToClients(int serverSocket, char* c2mShmpName);

// Starts a TCP server in the specified port with specified settings 
void startServer(int port, size_t maxCon);
#endif
