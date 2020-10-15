#ifndef SERVER_H
#define SERVER_H
#include "basic.h"

void initServer(int port, char root[]);
void* interact(void* sock);
int cmdCall(char cmd[], Connection* conn);
void errorCall(char cmd[], Connection* conn);

#endif
