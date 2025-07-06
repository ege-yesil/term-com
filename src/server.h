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
#include <fcntl.h>
#define BUF_SIZE 1024

enum commandType {
    SAY,
    KICK 
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

void listenToClients(int serverSocket);
// Starts a TCP server in the specified port with specified settings 
void startServer(int port, size_t maxCon);
#endif
