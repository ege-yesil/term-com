#ifndef UTIL_H
#define UTIL_H
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <fcntl.h>

#define FILE_CLOSED "\r>FD_CLOSED"

void sendResponse(int socket, char* str);
// use free() to free any char* created with this function 
// a nonblocking way of reading any file descriptor
char* readFile(int file);
#endif
