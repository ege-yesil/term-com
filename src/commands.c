#include "commands.h"

void exec_say(struct shmpBuf* shmp, char* str) {
    shmp->cnts = strlen(str);
    shmp->type = SAY;
    strcpy(shmp->buf, str);
    shmp->id++;
    __sync_synchronize();
}


void exec_kick(struct shmpBuf* shmp, char* who,char* str) {
    shmp->cnts = strlen(str);
    shmp->type = KICK;
    strcpy(shmp->buf, str);
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
