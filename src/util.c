#include "util.h"

void sendResponse(int socket, char* str) {
    char* strFinal = malloc(strlen(str) + 5);
    if (!strFinal) {
        fprintf(stderr, "Could not reallocate string to send as response\nError string: %s", strerror(errno));
        return;
    }
    memcpy(strFinal, str, strlen(str));
    strcpy(strFinal + strlen(str), "<END>\r");
    write(socket, strFinal, strlen(strFinal));
}

char* getResponse(int socket) {
    size_t bufferSize = 1024;
    char* buffer = malloc(sizeof(char) * bufferSize);
    size_t bytes = -1;
    size_t responseSize = 0;
    while (bytes > 0) {
        if (responseSize > bufferSize) {
            bufferSize = responseSize;
            buffer = realloc(buffer, bufferSize);
        }
        bytes = read(socket, buffer + (responseSize * sizeof(char)), bufferSize);
        responseSize += bytes;
        char* pos = strstr(buffer, "<END>\r");
        if (pos != NULL) {
            size_t realSize = pos - buffer;
            char* o = malloc(realSize);
            memcpy(o, buffer, realSize);
            free(buffer);
            return o;
        }
    }
 
    if (bytes < 0) {
        fprintf(stderr, "read function failed\nError string: %s\n", strerror(errno));
        free(buffer);
    }  
 
    return NULL;
}

