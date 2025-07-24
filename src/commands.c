#include "commands.h"

void resetCommandMem(struct commandMem* cmd) {
    pthread_mutex_lock(&cmd->lock);
    cmd->newestCommand = 0;
    for (size_t i = 0; i < MAX_CMD_SIZE; i++) {
        cmd->commands[i].cnts = 0;
        cmd->commands[i].type = NOCMD;
        *cmd->commands[i].buf = '\0';
    }
    pthread_mutex_unlock(&cmd->lock);
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
    char** seg = calloc(segSize, sizeof(char));
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

void exec_say(struct commandMem* cmd, char* str) {
    if (str == NULL) return;
    
    pthread_mutex_lock(&cmd->lock);
    cmd->newestCommand++;
    cmd->commands[cmd->newestCommand].cnts = strlen(str);
    cmd->commands[cmd->newestCommand].type = SAY;
    strcpy(*cmd->commands[cmd->newestCommand].buf, str);
    pthread_mutex_unlock(&cmd->lock);    
}

void exec_kick(struct commandMem* cmd, char* who, char* str) {
    if (str == NULL) return;
    pthread_mutex_lock(&cmd->lock);
    cmd->newestCommand++;
    size_t whoSize = strlen(who);
    size_t strSize = strlen(str);
    cmd->commands[cmd->newestCommand].cnts = whoSize + strSize; 
    if (cmd->commands[cmd->newestCommand].cnts > BUF_SIZE) {
        cmd->commands[cmd->newestCommand].cnts = 0;
        cmd->newestCommand--;
        printf("Too big kick string. Couldnt execute command");
        return;
    }
    cmd->commands[cmd->newestCommand].type = KICK;
    strcpy(*cmd->commands[cmd->newestCommand].buf, who);
    *cmd->commands[cmd->newestCommand].buf[whoSize] = '\r';
    strcpy(*cmd->commands[cmd->newestCommand].buf + whoSize + 1, str);
    pthread_mutex_unlock(&cmd->lock);
}


void exec_kickAll(struct commandMem* cmd, char* str) {
    if (str == NULL) return; 
    
    pthread_mutex_lock(&cmd->lock);
    cmd->newestCommand++;
    cmd->commands[cmd->newestCommand].cnts = strlen(str);
    cmd->commands[cmd->newestCommand].type = KICKALL;
    strcpy(*cmd->commands[cmd->newestCommand].buf, str);
    pthread_mutex_unlock(&cmd->lock);    
}
