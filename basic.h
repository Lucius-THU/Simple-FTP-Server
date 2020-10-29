#ifndef BASIC_H
#define BASIC_H
#include <sys/stat.h>
#include <netinet/in.h>
#define BUF_SIZE 1024

// 定义连接的状态
typedef enum Auth{
    notLogin, normal, needTransferConn
} Auth;

// 连接的信息
typedef struct Connection{
    int sock, dataSock, preUser, preRnfr, isPasv;
    long offset;
    struct sockaddr_in addr;
    Auth auth;
    char ip[BUF_SIZE], username[BUF_SIZE], root[BUF_SIZE], dir[BUF_SIZE], rnfrDir[BUF_SIZE];
} Connection;

int createSocket(int ip, int* port); // 创建服务端 socket
void setPortSocket(char cmd[], Connection* conn); // 设置 PORT 模式的目标 socket
int getPortSocket(Connection* conn, int* sock); // 获取数据连接所使用的 socket
void encodePath(char path[]); // 整理路径
int lenOfCmd(char cmd[]); // 去掉指令末尾的换行符，返回长度
void getPath(Connection* conn, char dir[], char fullDir[], char cmd[]); // 根据输入路径和当前路径，设置目标路径
int sendInfo(const char name[], struct stat* fileStat, int sock); // LIST 指令编码文件信息

#endif
