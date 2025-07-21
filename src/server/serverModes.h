#ifndef SERVER_MODES_H
#define SERVER_MODES_H

#include <stdlib.h>
#include <stdio.h>

#include "../util.h"

void silience(int client, const char* in);
void echo(int client, const char* in);

#endif
