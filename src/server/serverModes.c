#include "serverModes.h"

void silience(int client, const char* in) {
    return;
}

void echo(int client, const char* in) {
    sendResponse(client, in);
}
