#include "basic.h"
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <unistd.h>

static char msg[BUF_SIZE];

int createSocket(int ip, int* port){
    int sock;
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(*port);
	addr.sin_addr.s_addr = htonl(ip);
    if((sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1){
		printf("Error socket(): %s(%d)\n", strerror(errno), errno);
		exit(EXIT_FAILURE);
	}
    while(bind(sock, (struct sockaddr*)&addr, sizeof(addr)) == -1){
        *port = *port + 1;
        addr.sin_port = htons(*port);
	}
    if(listen(sock, 10) == -1){
		printf("Error listen(): %s(%d)\n", strerror(errno), errno);
		exit(EXIT_FAILURE);
	}
    return sock;
}

void setPortSocket(char cmd[], Connection* conn){
    int len = lenOfCmd(cmd);
    cmd[len] = 0;
    char* p = cmd;
    char ip[20] = "";
    for(int i = 0; i < 4; i++){
        if(i) strcat(ip, ".");
        strncat(ip, p, strchr(p, ',') - p);
        p = strchr(p, ',') + 1;
    }
    char* f = strchr(p, ',');
    *f = 0;
    int port = atoi(p) * 256 + atoi(f + 1);
    if((conn->dataSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1){
		printf("Error socket(): %s(%d)\n", strerror(errno), errno);
		exit(EXIT_FAILURE);
	}
    memset(&(conn->addr), 0, sizeof(conn->addr));
	conn->addr.sin_family = AF_INET;
	conn->addr.sin_port = htons(port);
	if(inet_pton(AF_INET, ip, &(conn->addr.sin_addr)) <= 0){
		printf("Error inet_pton(): %s(%d)\n", strerror(errno), errno);
		exit(EXIT_FAILURE);
	}
}

int getPortSocket(Connection* conn, int* sock){
    if(conn->isPasv){
        return *sock = accept(conn->dataSock, NULL, NULL);
    }
    *sock = conn->dataSock;
    return connect(*sock, (struct sockaddr*)&(conn->addr), sizeof(conn->addr));
}

void encodePath(char path[]){
    char s[BUF_SIZE] = "";
    int len = strlen(path);
    path[len] = '/';
    path[len + 1] = 0;
    char* l = strchr(path, '/');
    while(l != NULL){
        char* r = strchr(l + 1, '/');
        if(r == NULL) break;
        if(r == l + 1 || (r == l + 2 && l[1] == '.')){
            l = r;
            continue;
        }
        if(r == l + 3 && l[1] == '.' && l[2] == '.'){ // deal with "../"
            char* t = strrchr(s, '/');
            if(t != NULL) t[0] = 0;
        } else strncat(s, l, r - l);
        l = r;
    }
    if(!s[0]) strcpy(s, "/");
    strcpy(path, s);
}

int lenOfCmd(char cmd[]){
    char* r = strchr(cmd, '\r');
    char* n = strchr(cmd, '\n');
    int len = strlen(cmd);
    if(r != NULL) len = r - cmd;
    if(n != NULL) len = len > n - cmd ? n - cmd: len;
    return len;
}

void getPath(Connection* conn, char dir[], char fullDir[], char cmd[]){
    int len = lenOfCmd(cmd);
    if(cmd[0] == '/') strcpy(dir, cmd);
    else {
        strcpy(dir, conn->dir);
        strcat(dir, "/");
        strncat(dir, cmd, len);
    }
    encodePath(dir);
    strcpy(fullDir, conn->root);
    strcat(fullDir, dir);
}

int sendInfo(const char name[], struct stat* fileStat, int sock){
    strcpy(msg, "+");
    char size[] = "s%d,";
    char time[] = "m%d,";
    char iden[] = "i%d.%d,\t";
    if(S_ISREG(fileStat->st_mode)) strcat(msg, "r,");
    if(S_ISDIR(fileStat->st_mode)) strcat(msg, "/,");
    sprintf(msg + strlen(msg), size, fileStat->st_size);
    sprintf(msg + strlen(msg), time, fileStat->st_mtime);
    sprintf(msg + strlen(msg), iden, fileStat->st_dev, fileStat->st_ino);
    strcat(msg, name);
    strcat(msg, "\r\n");
    return write(sock, msg, strlen(msg));
}
