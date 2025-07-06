#ifndef UTIL_H
#define UTIL_H
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>

void sendResponse(int socket, char* str);
//use free() to free any char* created with this function 
char* getResponse(int socket);
#endif
