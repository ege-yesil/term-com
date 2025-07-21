#ifndef UTIL_H
#define UTIL_H
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <sys/mman.h>

#define FILE_CLOSED "\r>FD_CLOSED"

void* createSharedMem(const char* path, int shmFlags, int mmapFlags, size_t size);

void sendResponse(int socket, const char* str);
// use free() to free any char* created with this function 
// a nonblocking way of reading any file descriptor
char* readFile(int file);
#endif
