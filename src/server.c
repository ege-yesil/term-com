#include "server.h"

void manageClient(int client, struct sockaddr_in addr, char* c2mShmpName) { 
    int shmfd = shm_open(c2mShmpName, O_RDONLY, 0);
    
    struct shmpBuf* shmp;
    if (shmfd == -1) {
        fprintf(stderr, "Could not create shared memory fd for the manager\n");
        exit(1);
    }
    if ((shmp = mmap(NULL, sizeof(struct shmpBuf), PROT_READ, MAP_SHARED, shmfd, 0)) == MAP_FAILED) {
        fprintf(stderr, "Manager memory map to the shared memory space failed\nError string: %s\n", strerror(errno));
        exit(1);
    }
    size_t lastReqID = 0;
    while (1) {
        if (recv(client, NULL, 1, MSG_DONTWAIT | MSG_DONTWAIT) == 0) {
            printf("Connection with client %s lost. Ending thread\n", inet_ntoa(addr.sin_addr));
            exit(0);
        }

        char* response = readFile(client);
        if (response != NULL) {
            printf("Client %s said: \"%s\" \n", inet_ntoa(addr.sin_addr), response);
            free(response);
        }

        if (shmp->id > lastReqID) {
            __sync_synchronize();
            if (shmp->type == SAY) {
                printf("Executing SAY command with buffer: \"%s\" to client: %s\n", shmp->buf, inet_ntoa(addr.sin_addr));
                sendResponse(client, shmp->buf);
            }
            if (shmp->type == KICKALL) {
                printf("Executing KICK command with buffer: \"%s\" to client: %s\n", shmp->buf, inet_ntoa(addr.sin_addr));
                sendResponse(client, shmp->buf);
                close(client);
                exit(0);
            }
            lastReqID = shmp->id;
        } 
        usleep(100000);
    }
    munmap(shmp, sizeof(struct shmpBuf));
}

char** parseCommand(size_t segSize) {
    char* temp = malloc(1024);
    char** seg = malloc(sizeof(char*) * segSize);
    size_t currentSegment = 0;
    size_t readBytes = 0;
    
    bool str = false;
    size_t bytes = read(STDIN_FILENO, temp, 1024);
    for (int i = 0; i < bytes; i++) { 
        if (temp[i] == '"') {
            str = !str;
            
            if (str) {
                readBytes++;
                continue;
            } 
        }

        if ((temp[i] == ' ' || temp[i] == '"' || temp[i] == '\n') && !str && i - readBytes > 0) {
            if (currentSegment > segSize) {
                printf("Too many arguments - MAX %d arguments allowed", segSize);
                fflush(stdout);
                break;
            }
            
            size_t len = i - readBytes;
            seg[currentSegment] = malloc(len + 1);
            memcpy(seg[currentSegment], temp + readBytes, len);
            seg[currentSegment][len] = '\0';
            
            readBytes = i + 1; // to not count the ' '
            currentSegment++;
        } else if(temp[i] == ' ' && !str && i - readBytes == 0) {
            readBytes++;
        }
    }
    free(temp);
    return seg;
}

void manageCommands(char* c2mShmpName) {
    int shmfd = shm_open(c2mShmpName, O_RDWR | O_CREAT, 0600);
    struct shmpBuf* shmp;
    if (shmfd == -1) {
        fprintf(stderr, "Could not create shared memory space for the commander\n");
        exit(1);
    }
    if (ftruncate(shmfd, sizeof(struct shmpBuf)) == -1) {
        fprintf(stderr, "Could not truncate shared memory space to desired size\n");
        exit(1);
    }
    if ((shmp = mmap(NULL, sizeof(struct shmpBuf), PROT_READ | PROT_WRITE, MAP_SHARED, shmfd, 0)) == MAP_FAILED) {
        fprintf(stderr, "Commander memory map to the shared memory space failed\n");
        exit(1);
    }
    shmp->id = 0;
    shmp->cnts = 0;
    
    size_t comID = 1; 
    while (1) {
        char** seg = parseCommand(32);
        if (strcmp(seg[0], "say") == 0) {
            exec_say(shmp, seg[1]);
        }
        if (strcmp(seg[0], "kick") == 0) {
            exec_kick(shmp, seg[1], seg[2]);
        }
        if (strcmp(seg[0], "quit") == 0) {
            exec_kickAll(shmp, seg[1]);
        }
        //CLEANUP
        for (int i = 0; i < 32; i++) {
            if (seg[i] != NULL) { 
                free(seg[i]);
                seg[i] = NULL;
            }
        }
    }
    munmap(shmp, sizeof(struct shmpBuf));
}

void listenToClients(int serverSocket, char* c2mShmpName) {
    struct sockaddr_in* clientSockAddr = malloc(sizeof(struct sockaddr_in));
    socklen_t clientSockLen = (socklen_t)sizeof(*clientSockAddr);
    
    pid_t commander = fork();
    if (commander == 0) manageCommands(c2mShmpName);

    while (1) {
        int clientSocket = accept(serverSocket, (struct sockaddr*)clientSockAddr, &clientSockLen);
        //let the parent deal with accepting requests and the child with reading and responding
        pid_t manager = fork();
        if (manager == 0) manageClient(clientSocket, *clientSockAddr, c2mShmpName);
    }
    free(c2mShmpName);
    free(clientSockAddr);
}

void startServer(int port, size_t maxCon) {
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        fprintf(stderr, "Could not start server\nError string: %s\n", strerror(errno));
        exit(1);
    }

    struct sockaddr_in* address = malloc(sizeof(struct sockaddr_in));
    address->sin_family = AF_INET;
    address->sin_port = htons(port);
    address->sin_addr.s_addr = htonl(INADDR_ANY); 

    if (bind(sockfd, (const struct sockaddr*)address, (socklen_t)sizeof(*address)) ==  -1) {
        fprintf(stderr, "Could not bind the server socket\nError string: %s\n", strerror(errno));
        exit(1);
    }
    
    if (listen(sockfd, maxCon) < 0)  {
        fprintf(stderr, "Could not start listening\nErrorString: %s\n", strerror(errno));
        exit(1);
    }

    printf("Server setup complete\nWaiting for clients\n");
    listenToClients(sockfd, "/commanderToManager");
}

int main(int argc, char* argv[]) {
    int port = 0;
    size_t maxCon = 128;
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--port") == 0 || strcmp(argv[i], "-p") == 0) {  
            if (++i >= argc) {
                fprintf(stderr, "Wrong use of port argument (-p or --port)\nNo port specified\n");
                printf("Defaulting to port 8080\n");
                continue;
            }
            char* endptr;
            port = strtol(argv[i], &endptr, 10);
            if (endptr == argv[i]) {
                fprintf(stderr, "Wrong use of port argument (-p or --port)\nSpecified port is not a number\n");
                printf("Letting the system choose the port\n");
                port = 0;
                continue;
            } else if (*endptr != '\0') {
                fprintf(stderr, "Wrong use of port argument (-p or --port)\nInvalid char: %c\n", *endptr);  
                printf("Letting the system choose the port\n");
                port = 0;
                continue;
            }
            continue;
        }

        if (strcmp(argv[i], "--maxPending") == 0 || strcmp(argv[i], "-mp") == 0) {
            if (++i >= argc) {
                fprintf(stderr, "Wrong use of maximum pending argument (-mp or --maxPen)\nNo size specified\n");
                printf("Defaulting maximum pending connections to 128\n");
                continue;
            }
            char* endptr;
            maxCon = strtol(argv[i], &endptr, 10);
            if (endptr == argv[i]) {
                fprintf(stderr, "Wrong use of maximum pending argument (-mp or --maxPen)\nSpecified size is not a number\n");
                printf("Defaulting to 128\n");
                maxCon = 128;
                continue;
            } else if (*endptr != '\0') {
                fprintf(stderr, "Wrong use of maximum pending argument (-mp or --maxPen)\nInvalid char: %c\n", *endptr);
                printf("Defaulting to 128\n");
                maxCon = 128;
                continue;
            }
            continue;
        }
    }
   
    printf("Starting server on port: %d\n", port);
    startServer(port, maxCon);    

    return 0;
}
