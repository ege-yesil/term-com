#include "server.h"

void changeServerResponseFn(size_t handlerIndex) {
    size_t* shmp = createSharedMem("/responseFptr", O_RDWR | O_CREAT, PROT_WRITE | PROT_READ, sizeof(size_t));
    *shmp = handlerIndex;
    munmap(shmp, sizeof(int*));
}

bool executeCommand(int client, struct sockaddr_in addr, struct command cmd) {
    if (cmd.type == NOCMD) return true;
    printf("Executing %s command on client %s\n", cmdTypeToString(cmd.type), inet_ntoa(addr.sin_addr));
    fflush(stdout);
    switch (cmd.type) {
        case SAY:
            sendResponse(client, cmd.buf);
            break;
        case KICK:
            size_t ipSize = 16; // maximum address can be 16 chars
            char* ip = malloc(ipSize);
            size_t i = 0;
            while (cmd.buf[i] != '\r') {
                ip[i] = cmd.buf[i];
                i++;
            }
            ip[i] = '\0';
            if (ipSize > strlen(ip)) ip = realloc(ip, strlen(ip) + 1); // +1 for null terminator
            if (strcmp(inet_ntoa(addr.sin_addr), ip) == 0) {
                char*  response = malloc(strlen(KICK_MESSAGE) + strlen(cmd.buf + i));
                strcpy(response, KICK_MESSAGE);
                strcpy(response + strlen(KICK_MESSAGE), cmd.buf + i + 1);   
                
                sendResponse(client, response);  // +1 to not send \r
            }
            break;
        case KICKALL:
            char*  response = malloc(strlen(KICK_MESSAGE) + strlen(cmd.buf));
            strcpy(response, KICK_MESSAGE);
            strcpy(response + strlen(KICK_MESSAGE), cmd.buf);   
            
            sendResponse(client, response);
            break;
    }
    return true;
}

static void manageClient(int client, struct sockaddr_in addr) { 
    struct commandShm* cmdShmp = createSharedMem("/commanderToManager", O_RDONLY, PROT_READ, sizeof(struct commandShm));
    size_t* responseIndex = createSharedMem("/responseFptr", O_RDONLY, PROT_READ, sizeof(size_t)); 
   
    size_t lastCmdID = cmdShmp->newestCommand;
    while (1) {
        const char* response = readFile(client);
        if (response != NULL) {
            if (strcmp(response, FILE_CLOSED) == 0) {
                printf("Connection with client %s lost. Ending thread\n", inet_ntoa(addr.sin_addr));
                munmap(cmdShmp, sizeof(struct commandShm));
                exit(0);
            }
            handlers[*responseIndex](client, response); 

            printf("Client %s said: \"%s\" \n", inet_ntoa(addr.sin_addr), response);
            free((char*)response);
        }
        
        if (cmdShmp->newestCommand == 0) lastCmdID = 0;
        if (lastCmdID < cmdShmp->newestCommand) {
            bool cmdSuccess = executeCommand(client, addr, cmdShmp->commands[cmdShmp->newestCommand]);
            enum commandType type = cmdShmp->commands[cmdShmp->newestCommand].type;
            if (type == KICK || type == KICKALL) {
                munmap(cmdShmp, sizeof(struct commandShm));
                shutdown(client, SHUT_RDWR);
                close(client);
                exit(0);
            }
            lastCmdID++;
        }

        usleep(100000);
    }
    munmap(cmdShmp, sizeof(struct commandShm));
}

static void manageCommands() {
    struct commandShm* shmp = createSharedMem("/commanderToManager", O_RDWR | O_CREAT, PROT_READ | PROT_WRITE, sizeof(struct commandShm));
    
    resetCommandShm(shmp); 
    size_t comID = 1; 
    while (1) {
        if (shmp->newestCommand >= MAX_CMD_SIZE - 1)
            resetCommandShm(shmp);
        
        char** seg = parseCommand(32);

        if (strcmp(seg[0], "say") == 0) {
            exec_say(shmp, seg[1]);
        }
        if (strcmp(seg[0], "kick") == 0) {
            exec_kick(shmp, seg[1], seg[2]);
        }
        if (strcmp(seg[0], "kickall") == 0) {
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
    munmap(shmp, sizeof(struct commandShm));
}

void listenToClients(int serverSocket) {
    struct sockaddr_in* clientSockAddr = malloc(sizeof(struct sockaddr_in));
    socklen_t clientSockLen = (socklen_t)sizeof(*clientSockAddr);
     
    pid_t commander = fork();
    if (commander == 0) manageCommands();

    changeServerResponseFn(1); // all response functions are defined in the serverModes.c
    while (1) {
        int clientSocket = accept(serverSocket, (struct sockaddr*)clientSockAddr, &clientSockLen);
        //let the parent deal with accepting requests and the child with reading and responding
        pid_t manager = fork();
        if (manager == 0) manageClient(clientSocket, *clientSockAddr);
    }
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
    listenToClients(sockfd); 
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
