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

static char rootPath[BUF_SIZE];

void initServer(int port, char root[]){
    int sock = createSocket(INADDR_ANY, port);
    if(access(root, F_OK) == -1){
        printf("Error: '%s' does not exist.\n", root);
		exit(EXIT_FAILURE);
    }
    strcpy(rootPath, root);

    while(1){
        int* conn = (int*)malloc(sizeof(int));
        if((*conn = accept(sock, NULL, NULL)) == -1){
			printf("Error accept(): %s(%d)\n", strerror(errno), errno);
            free(conn);
			continue;
		} else {
            pthread_t pid;
            if(pthread_create(&pid, NULL, interact, conn)){
                printf("Error pthread_create(): %s(%d)\n", strerror(errno), errno);
                exit(EXIT_FAILURE);
            }
        }
    }
    close(sock);
}

void* interact(void* sock){
    Connection* conn = (Connection*)malloc(sizeof(Connection));
    conn->sock = *(int*)sock;
    conn->auth = notLogin;
    strcpy(conn->root, rootPath);
    strcpy(conn->dir, "/");
    free(sock);
    char msg[BUF_SIZE] = "220 Anonymous FTP server ready.\r\n";
    if(write(conn->sock, msg, strlen(msg)) < 0){
        printf("Error write(): %s(%d)\n", strerror(errno), errno);
        exit(EXIT_FAILURE);
    }
    while(1){
        int len = read(conn->sock, msg, BUF_SIZE);
        if(len < 0){
            printf("Error read(): %s(%d)\n", strerror(errno), errno);
            exit(EXIT_FAILURE);
        }
        msg[len] = 0;
        if(len) cmdCall(msg, conn);
    }
    close(conn->sock);
    free(conn);
    return NULL;
}

void cmdCall(char cmd[], Connection* conn){
    int flag = 0;
    for(int i = 0; i < CMD_CNT; i++){
        if(cmds[i].auth <= conn->auth && !strncmp(cmds[i].prefix, cmd, cmds[i].len)){
            cmds[i].func(cmd + cmds[i].len, conn);
            flag = 1;
            break;
        }
    }
    if(!flag){
        errorCall(cmd, conn);
    }
}

void errorCall(char cmd[], Connection* conn){
    char msg[BUF_SIZE];
    if(conn->auth == notLogin && !strncmp(cmd, "PASS ", 5)){
        strcpy(msg, "503 The previous request was not USER.\r\n");
    }
    write(conn->sock, msg, strlen(msg));
}
