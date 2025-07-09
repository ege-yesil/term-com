#include "commands.h"

char* comTypeToString(enum commandType type) {
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

void exec_say(struct shmpBuf* shmp, char* str) {
    shmp->cnts = strlen(str);
    shmp->type = SAY;
    strcpy(shmp->buf, str);
    shmp->id++;
    __sync_synchronize();
}


void exec_kick(struct shmpBuf* shmp, char* who, char* str) {
    size_t whoSize = strlen(who);
    size_t strSize = strlen(str);
    shmp->cnts = whoSize + strSize; 
    if (shmp->cnts > BUF_SIZE) {
        shmp->cnts = 0;
        printf("Too big kick string. Couldnt execute command");
        return;
    }
    shmp->type = KICK;
    strcpy(shmp->buf, who);
    shmp->buf[whoSize] = '\r';
    strcpy(shmp->buf + whoSize + 1, str);
    shmp->id++;
    __sync_synchronize();
}


void exec_kickAll(struct shmpBuf* shmp, char* str) {
    shmp->cnts = strlen(str);
    shmp->type = KICKALL;
    strcpy(shmp->buf, str);
    shmp->id++;
    __sync_synchronize();
}
