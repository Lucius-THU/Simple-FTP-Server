#include "server.h"
#include "cmd.h"
#include <sys/io.h>
#include <sys/socket.h>
#include <pthread.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

static char rootPath[BUF_SIZE]; // FTP 服务端的根目录

// 启动服务端
void initServer(int port, char root[]){
    int ps = port;
    int sock = createSocket(INADDR_ANY, &ps);
    if(ps != port){
        close(sock);
        printf("Error bind(): Address already in use(98)\n");
		exit(EXIT_FAILURE);
    }
    if(access(root, F_OK) == -1){
        printf("Error: '%s' does not exist.\n", root);
		exit(EXIT_FAILURE);
    }
    strcpy(rootPath, root);
    while(1){ // 循环接受客户端连接
        int* conn = (int*)malloc(sizeof(int));
        if((*conn = accept(sock, NULL, NULL)) == -1){
			printf("Error accept(): %s(%d)\n", strerror(errno), errno);
            free(conn);
			continue;
		} else { // 多线程
            pthread_t pid;
            if(pthread_create(&pid, NULL, interact, conn)){ // 创建线程
                printf("Error pthread_create(): %s(%d)\n", strerror(errno), errno);
                exit(EXIT_FAILURE);
            }
        }
    }
    close(sock);
}

// 在线程中处理客户端连接
void* interact(void* sock){
    Connection* conn = (Connection*)malloc(sizeof(Connection));
    conn->sock = *(int*)sock;
    conn->auth = notLogin;
    conn->offset = 0;
    strcpy(conn->root, rootPath);
    strcpy(conn->dir, "/");
    free(sock);
    char msg[BUF_SIZE] = "220 Anonymous FTP server ready.\r\n";
    if(write(conn->sock, msg, strlen(msg)) < 0){
        printf("Error write(): %s(%d)\n", strerror(errno), errno);
        exit(EXIT_FAILURE);
    }
    while(1){ // 循环处理指令
        int len = read(conn->sock, msg, BUF_SIZE);
        if(len < 0){
            printf("Error read(): %s(%d)\n", strerror(errno), errno);
            exit(EXIT_FAILURE);
        }
        msg[len] = 0;
        if(!len || cmdCall(msg, conn)) break;
    }
    if(conn->auth == needTransferConn && conn->isPasv) close(conn->dataSock);
    close(conn->sock);
    free(conn);
    return NULL;
}

// 匹配指令
int cmdCall(char cmd[], Connection* conn){
    for(int i = 0; i < CMD_CNT; i++){
        if(cmds[i].auth <= conn->auth && !strncmp(cmds[i].prefix, cmd, cmds[i].len)){ // 指令所要求的权限与当前连接的状态相匹配
            cmds[i].func(cmd + cmds[i].len, conn);
            return !strcmp(cmds[i].prefix, "QUIT");
        }
    }
    errorCall(cmd, conn);
    return 0;
}

// 处理异常指令
void errorCall(char cmd[], Connection* conn){
    char msg[BUF_SIZE];
    if(conn->auth == notLogin){
            strcpy(msg, "530 Permission denied.\r\n");
    } else if(conn->auth == normal){
        strcpy(msg, "425 Please use PORT or PASV request first.\r\n");
    }
    write(conn->sock, msg, strlen(msg));
}
