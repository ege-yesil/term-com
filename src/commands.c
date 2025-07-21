#include "commands.h"

void resetCommandShm(struct commandShm* shmp) {
    shmp->newestCommand = 0;
    for (size_t i = 0; i < MAX_CMD_SIZE; i++) {
        shmp->commands[i].cnts = 0;
        shmp->commands[i].type = NOCMD;
        *shmp->commands[i].buf = '\0';
    }
    __sync_synchronize();
}

char* cmdTypeToString(enum commandType type) {
    switch(type) {
        case SAY:
            return "SAY";
        case KICK:
            return "KICK";
        case KICKALL:
            return "KICKALL";
        default:
            return "NOCOM";
    }
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
            
            readBytes = i + 1; // +1 to not count the ' '
            currentSegment++;
        } else if(temp[i] == ' ' && !str && i - readBytes == 0) {
            readBytes++;
        }
    }
    free(temp);
    return seg;
}

void exec_say(struct commandShm* shmp, char* str) {
    if (str == NULL) return;
    shmp->newestCommand++;
    shmp->commands[shmp->newestCommand].cnts = strlen(str);
    shmp->commands[shmp->newestCommand].type = SAY;
    strcpy(shmp->commands[shmp->newestCommand].buf, str);
    
    __sync_synchronize();
}

void exec_kick(struct commandShm* shmp, char* who, char* str) {
    if (str == NULL) return;
    shmp->newestCommand++;
    size_t whoSize = strlen(who);
    size_t strSize = strlen(str);
    shmp->commands[shmp->newestCommand].cnts = whoSize + strSize; 
    if (shmp->commands[shmp->newestCommand].cnts > BUF_SIZE) {
        shmp->commands[shmp->newestCommand].cnts = 0;
        shmp->newestCommand--;
        printf("Too big kick string. Couldnt execute command");
        return;
    }
    shmp->commands[shmp->newestCommand].type = KICK;
    strcpy(shmp->commands[shmp->newestCommand].buf, who);
    shmp->commands[shmp->newestCommand].buf[whoSize] = '\r';
    strcpy(shmp->commands[shmp->newestCommand].buf + whoSize + 1, str);
    
    __sync_synchronize();
}


void exec_kickAll(struct commandShm* shmp, char* str) {
    if (str == NULL) return; 
    shmp->newestCommand++;
    shmp->commands[shmp->newestCommand].cnts = strlen(str);
    shmp->commands[shmp->newestCommand].type = KICKALL;
    strcpy(shmp->commands[shmp->newestCommand].buf, str);
    
    __sync_synchronize();
}
