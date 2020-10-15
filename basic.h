#ifndef BASIC_H
#define BASIC_H
#include <sys/stat.h>
#include <netinet/in.h>
#define BUF_SIZE 1024

typedef enum Auth{
    notLogin, normal, needTransferConn
} Auth;

typedef struct Connection{
    int sock, dataSock, preUser, preRnfr, isPasv;
    long offset;
    struct sockaddr_in addr;
    Auth auth;
    char ip[BUF_SIZE], username[BUF_SIZE], root[BUF_SIZE], dir[BUF_SIZE], rnfrDir[BUF_SIZE];
} Connection;

int createSocket(int ip, int* port);
void setPortSocket(char cmd[], Connection* conn);
int getPortSocket(Connection* conn, int* sock);
void encodePath(char path[]);
int lenOfCmd(char cmd[]);
void getPath(Connection* conn, char dir[], char fullDir[], char cmd[]);
int sendInfo(const char name[], struct stat* fileStat, int sock);

#endif
