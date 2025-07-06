#include "client.h"
#include "util.h"

void listenToServer(int socket) {
    sendResponse(socket, "Hello");

    while (1) {
        char* response = getResponse(socket);
        if (response != NULL) {
            printf("Server said: %s\n", response);
            free(response);
        }
    }
    close(socket);
}

void startClient(struct in_addr conAddress, int port) {
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        fprintf(stderr, "Could not create socket\nError string: %s\n", strerror(errno));
        exit(1);
    }
    struct sockaddr_in* addr = malloc(sizeof(struct sockaddr_in));
    addr->sin_family = AF_INET;
    addr->sin_port = htons(port);
    addr->sin_addr = conAddress;

    if (connect(sockfd, (struct sockaddr*)addr, (socklen_t)sizeof(*addr)) == -1) {
        fprintf(stderr, "Could not connect the socket to specified address\nError string: %s\n", strerror(errno));
        exit(1);
    }
    listenToServer(sockfd);
}

int main(int argc, char* argv[]) {
    struct in_addr conAddress;
    inet_aton("127.0.0.1", &conAddress);
    int port = 8080;
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-ip") == 0) {
            if (++i >= argc) {
                fprintf(stderr, "Wrong use of ip argument (-ip)\nNo ip specified\n");
                printf("Defaulting to  127.0.0.1(localhost)");
                continue;
            }

            if (inet_aton(argv[i], &conAddress) == 0) {
                fprintf(stderr, "Wrong use of ip argument (-ip)\nInvalid ip\n");
                printf("Defaulting to 127.0.0.1(localhost)");
                inet_aton("127.0.0.1", &conAddress);
                continue;
            }
        }

        if (strcmp(argv[i], "--port") == 0 || strcmp(argv[i], "-p") == 0 ) {
            if (++i >= argc) {
                fprintf(stderr, "Wrong use of port argument (--port or -p)\nNo port specified\n");
                printf("Defaulting to port 8080\n");
                continue;
            }

            char* endptr;
            port = strtol(argv[i], &endptr, 10);
            if (endptr == argv[i]) {
                fprintf(stderr, "Wrong use of port argument (-p or --port)\nSpecified port is not a number\n");
                printf("Defaulting to port 8080\n");
                port = 8080;
                continue;
            } else if (*endptr != '\0') {
                fprintf(stderr, "Wrong use of port argument (-p or --port)\nInvalid char: %c\n", *endptr);
                printf("Defaulting to port 8080\n");
                port = 8080;
                continue;
            }
        }
    }
    startClient(conAddress, port);
    return 0;
}
