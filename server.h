#ifndef SERVER_H
#define SERVER_H
#include "basic.h"

void initServer(int port, char root[]); // 启动服务端
void* interact(void* sock); // 处理客户端连接
int cmdCall(char cmd[], Connection* conn); // 处理指令
void errorCall(char cmd[], Connection* conn); // 处理异常指令

#endif
