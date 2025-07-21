#include "util.h"

void* createSharedMem(const char* path, int shmFlags, int mmapFlags, size_t size) {
    int shmfd = shm_open(path, shmFlags, 0600);
    if (shmfd == -1) { 
        fprintf(stderr, "Could not create shared memory space\n");
        return NULL;
    }
    if (shmFlags & O_RDWR && ftruncate(shmfd, size) == -1) {
        fprintf(stderr, "Could not truncate shared memory space to desired size\n");
        return NULL; 
    }
    void* shmp = malloc(size);
    if ((shmp = mmap(NULL, size, mmapFlags, MAP_SHARED, shmfd, 0)) == MAP_FAILED) {
        fprintf(stderr, "Memory map to the shared memory space failed");
        free(shmp); 
        return NULL;
    }
    return shmp;
}

void sendResponse(int socket, const char* str) {
    write(socket, str, strlen(str));
}

char* readFile(int file) {
    fcntl(file, F_SETFL, fcntl(file, F_GETFL) | O_NONBLOCK);

    size_t bufferSize = 1024;
    char* buffer = malloc(bufferSize);
    ssize_t bytes = -1;
    size_t responseSize = 0;
    while (bytes !=  0) {
        bytes = read(file, buffer + responseSize, 1024);
        if (bytes > 0) responseSize += bytes;
        if (responseSize + 1024 >= bufferSize) {
            bufferSize *= 2;
            buffer = realloc(buffer, bufferSize);
            if (buffer == NULL) {
                fprintf(stderr, "Could not reallocate read buffer\nError string: %s\n", strerror(errno));
                free(buffer);
                return NULL;
            }
        }

        if (bytes == -1 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
            if (responseSize == 0) {
                free(buffer);
                return NULL;
            }
            buffer = realloc(buffer, responseSize + 1);
            if (buffer == NULL) {
                fprintf(stderr, "Could not reallocate read buffer\nError string: %s\n", strerror(errno));
                free(buffer);
                return NULL;
            }

            buffer[responseSize] = '\0';
            return buffer;
        } else if (bytes == -1) {
            fprintf(stderr, "Read function failed\nError string: %s\n", strerror(errno));
            free(buffer);
            return NULL;
        }
    }
    // If program reached here bytes == 0 so file closed
    return FILE_CLOSED;
}

