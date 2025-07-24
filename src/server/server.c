#include "server.h"

static struct program programInstance;

struct program getProgram() {
    return programInstance;
}

bool executeCommand(struct clientData data, struct command cmd) {
    if (cmd.type == NOCMD) return true;
    printf("Executing %s command client %s\n", cmdTypeToString(cmd.type), inet_ntoa(data.addr.sin_addr));
    fflush(stdout);
    switch (cmd.type) {
        case SAY:
            sendResponse(data.client, cmd.buf[1]);
            break;
        case KICK:
            size_t ipSize = 16; // maximum address can be 16 chars
            char* ip = malloc(ipSize);
            size_t i = 0;
            while (cmd.buf[i] != '\r') {
                ip[i] = cmd.buf[1][i];
                i++;
            }
            ip[i] = '\0';
            if (ipSize > strlen(ip)) ip = realloc(ip, strlen(ip) + 1); // +1 for null terminator
            if (strcmp(inet_ntoa(addr.sin_addr), ip) == 0) {
                char*  response = malloc(strlen(KICK_MESSAGE) + strlen(cmd.buf + i));
                strcpy(response, KICK_MESSAGE);
                strcpy(response + strlen(KICK_MESSAGE), cmd.buf + i + 1);   
                
                sendResponse(clientData.client, response);  // +1 to not send \r
            }
            break;
        case KICKALL:
            char*  response = malloc(strlen(KICK_MESSAGE) + strlen(cmd.buf));
            strcpy(response, KICK_MESSAGE);
            strcpy(response + strlen(KICK_MESSAGE), cmd.buf);   
            
            sendResponse(clientData.client, response);
            break;
    }
    return true;
}

static void manageClient(void* arg) { 
    struct clientData data = (clientData)*arg;
    size_t lastCmdID = programInstance.commandMem.newestCommand;
    while (1) {
        const char* response = readFile(data.client);
        if (response != NULL) {
            if (strcmp(response, FILE_CLOSED) == 0) {
                printf("Connection with client %s lost. Ending thread\n", inet_ntoa(addr.sin_addr));
                munmap(cmdShmp, sizeof(struct commandShm));
                exit(0);
            }
            handlers[programInstance.state.responseIndex](client, response); 

            printf("Client %s said: \"%s\" \n", inet_ntoa(addr.sin_addr), response);
            free((char*)response);
        }
        
        if (cmdShmp->newestCommand == 0) lastCmdID = 0;
        if (lastCmdID < cmdShmp->newestCommand) {
            bool cmdSuccess = executeCommand((struct clientData){client, addr}, programInstance.cmd.commands[programInstance.cmd.newestCommand]);
            enum commandType type = programInstance.cmd.commands[cmd.newestCommand].type;
            if (type == KICK || type == KICKALL) {
                shutdown(client, SHUT_RDWR);
                close(client);
                exit(0);
            }
            lastCmdID++;
        }

        usleep(100000);
    }
}

static void manageCommands() {
    resetCommandMem(&programInstance.cmd); 

    size_t comID = 1; 
    while (1) {
        pthread_mutex_lock(&program.cmd.lock);
        if (program.cmd.newestCommand >= MAX_CMD_SIZE - 1)
            resetCommandMem(&programInstance.cmd);
        pthread_mutex_unlock(&program.cmd.lock);

        char** seg = parseCommand(32);

        if (strcmp(seg[0], "say") == 0) {
            exec_say(cmd, seg[1]);
        }
        if (strcmp(seg[0], "kick") == 0) {
            exec_kick(cmd, seg[1], seg[2]);
        }
        if (strcmp(seg[0], "kickall") == 0) {
            exec_kickAll(cmd, seg[1]);
        }
        //CLEANUP
        for (int i = 0; i < 32; i++) {
            if (seg[i] != NULL) { 
                free(seg[i]);
                seg[i] = NULL;
            }
        }
    }
}

void listenToClients(int serverSocket, size_t maxCon) {
    struct sockaddr_in* clientSockAddr = malloc(sizeof(struct sockaddr_in));
    socklen_t clientSockLen = (socklen_t)sizeof(*clientSockAddr);

    programInstance.messages = calloc(maxCon, sizeof(struct message));
    for (size_t i = 0; i < maxCon; i++) {
        int err = pthread_mutex_init(&programInstance.messages[i].lock, NULL);
        if (err != 0) {
            fprintf(stderr, "Could not  initialize mutex");
            exit(1);
        }
    }
    programInstance.state = {
        .connected = 0,
        .responseIndex = 0,
        .nextConnection = 0,
    };
    if (pthread_mutex_init(&programInstance.state.lock, NULL) != 0 ||
            pthread_mutex_init(&programInstance.cmd.lock, NULL) != 0) {
        fprintf(stderr, "Could not initialize mutex");
        exit(1);
    }
    

    pthread_t* threads = calloc(maxCon, sizeof(pthread_t));
    pthread_t commander;
    pthread_create(&commander, NULL, manageCommands, NULL);
    
    while (1) {
        int clientSocket = accept(serverSocket, (struct sockaddr*)clientSockAddr, &clientSockLen);
        pthread_mutex_lock(&programInstance.state.lock);
        programInstance.state.connected++;
        if (programInstance.state.connected > maxCon) {
            pthread_mutex_unlock(&programInstance.state.lock);
            sendResponse(clientSocket, "Server is full try to join at another time");
            shutdown(clientSocket, SHUT_RDWR);
            close(clientSocket);
            continue;
        }
        pthread_mutex_unlock(&programInstance.state.lock);      
        
        //let the parent deal with accepting requests and the child with reading and responding
        struct clientData data =  {
            .client = clientSocket,
            .addr = *clientSockAddr
        };
        int err = pthread_create(&threads[programInstance.state.nextConnection], NULL, manageClient, (void*)data);
        if (err != 0) {
            fprintf(stderr, "An error occured while creating thread");
        
            sendResponse(clientSocket, "Server is full try to join at another time");
            shutdown(clientSocket, SHUT_RDWR);
            close(clientSocket);
        }
    }
    free(clientSockAddr);
}

void startServer(int port, size_t maxPen, size_t maxCon) {
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
    
    if (listen(sockfd, maxPen) < 0)  {
        fprintf(stderr, "Could not start listening\nErrorString: %s\n", strerror(errno));
        exit(1);
    }

    printf("Server setup complete\nWaiting for clients\n");
    listenToClients(sockfd, maxCon); 
}

int main(int argc, char* argv[]) {
    int port = 0;
    size_t maxPen = 128;
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
                fprintf(stderr, "Wrong use of maximum pending argument (-mp or --maxPending)\nNo size specified\n");
                printf("Defaulting maximum pending connections to 128\n");
                continue;
            }
            char* endptr;
            maxPen = strtol(argv[i], &endptr, 10);
            if (endptr == argv[i]) {
                fprintf(stderr, "Wrong use of maximum pending argument (-mp or --maxPending)\nSpecified size is not a number\n");
                printf("Defaulting to 128\n");
                maxPen = 128;
                continue;
            } else if (*endptr != '\0') {
                fprintf(stderr, "Wrong use of maximum pending argument (-mp or --maxPending)\nInvalid char: %c\n", *endptr);
                printf("Defaulting to 128\n");
                maxPen = 128;
                continue;
            }
            continue;
        }

        if (strcmp(argv[i], "--maxConnection") ==  0 ||  strcmp(argv[i], "-mc") == 0) {
            if (++i >= argc) {
                fprintf(stderr, "Wrong use of maximum connection argument (-mc or --maxConnection)\nNo size specified\n");
                printf("Defaulting to maximum connection 128\n");
                continue;
            }
            char* endptr;
            maxCon = strtol(argv[i], &endptr, 10);
            if (endptr == argv[i]) {
                fprintf(stderr, "Wrong use of maximum connection argument (-mc or --maxConnection)\nSpecified size is not a number\n");
                printf("Defaulting to 128\n");
                maxCon = 128;
                continue;
            } else if (*endptr != '\0') {
                fprintf(stderr, "Wrong use of maximum connection argument (-mc or --maxConnection)\nInvalid char: %c\n", *endptr);
                printf("Defaulting to 128\n");
                maxPen = 128;
                continue;
            }
        }
            continue;
    }
   
    printf("Starting server on port: %d\n", port);
    startServer(port, maxPen, maxCon);

    return 0;
}
