#ifndef CLIENT_H
#define CLIENT_H
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

void listenToServer(int socket);
void startClient(struct in_addr conAddress, int port);
#endif
