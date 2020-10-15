#include "cmd.h"
#include <sys/io.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <dirent.h>
#include <sys/sendfile.h>
#define __USE_GNU
#include <fcntl.h>

static char listMsg[BUF_SIZE];
static char msg[BUF_SIZE];
static char dir[BUF_SIZE];
static char fullDir[BUF_SIZE];
static char name[BUF_SIZE];

const Cmd cmds[CMD_CNT] = {
    { notLogin, "USER ", 5, user },
    { notLogin, "PASS ", 5, pass },
    { notLogin, "SYST", 4, syst },
    { notLogin, "TYPE ", 5, type },
    { normal, "CWD ", 4, cwd },
    { normal, "PWD", 3, pwd },
    { normal, "MKD ", 4, mkd },
    { normal, "RMD ", 4, rmd },
    { normal, "RNFR ", 5, rnfr },
    { normal, "RNTO ", 5, rnto },
    { normal, "PORT ", 5, port },
    { normal, "PASV", 4, pasv },
    { needTransferConn, "LIST", 4, list },
    { needTransferConn, "RETR ", 5, retr },
    { needTransferConn, "STOR ", 5, stor }
};

void user(char cmd[], Connection* conn){
    conn->preUser = 1;
    conn->preRnfr = 0;
    strcpy(msg, "331 Please send your e-mail address as password.\r\n");
    strncpy(conn->username, cmd, lenOfCmd(cmd));
    write(conn->sock, msg, strlen(msg));
}

void pass(char cmd[], Connection* conn){
    if(conn->preUser){
        strcpy(msg, "230 Login successful.\r\n");
        conn->auth = normal;
    } else {
        strcpy(msg, "503 The previous request was not USER.\r\n");
    }
    conn->preUser = conn->preRnfr = 0;
    write(conn->sock, msg, strlen(msg));
}

void syst(char cmd[], Connection* conn){
    strcpy(msg, "215 UNIX Type: L8\r\n");
    conn->preUser = conn->preRnfr = 0;
    write(conn->sock, msg, strlen(msg));
}

void type(char cmd[], Connection* conn){
    if(!strncmp(cmd, "I", lenOfCmd(cmd))){
        strcpy(msg, "200 Type set to I.\r\n");
    } else {
        strcpy(msg, "504 The server does not support the parameter.\r\n");
    }
    conn->preUser = conn->preRnfr = 0;
    write(conn->sock, msg, strlen(msg));
}

void cwd(char cmd[], Connection* conn){
    getPath(conn, dir, fullDir, cmd);
    strcpy(fullDir, conn->root);
    strcat(fullDir, dir);
    if(access(fullDir, F_OK) == -1){
        strcpy(msg, "500 ");
        strncat(msg, cmd, lenOfCmd(cmd));
        strcat(msg, ": No such file or directory.\r\n");
    } else {
        strcpy(msg, "250 Okay.\r\n");
        strcpy(conn->dir, dir);
    }
    conn->preUser = conn->preRnfr = 0;
    write(conn->sock, msg, strlen(msg));
}

void pwd(char cmd[], Connection* conn){
    strcpy(msg, "257 \"");
    strcat(msg, conn->dir);
    strcat(msg, "\"\r\n");
    conn->preUser = conn->preRnfr = 0;
    write(conn->sock, msg, strlen(msg));
}

void mkd(char cmd[], Connection* conn){
    getPath(conn, dir, fullDir, cmd);
    strcat(fullDir, "/");
    if(access(fullDir, F_OK) == -1){
        for(char* i = strchr(fullDir + strlen(conn->root) + 1, '/'); i != NULL; i = strchr(i + 1, '/')){
            i[0] = 0;
            if(access(fullDir, F_OK) == -1){
                mkdir(fullDir, 0775);
            }
            i[0] = '/';
        }
        strcpy(msg, "257 \"");
        strcat(msg, dir);
        strcat(msg, "\" directory created.\r\n");
    } else {
        strcpy(msg, "521 \"");
        strcat(msg, dir);
        strcat(msg, "\" directory already exists.\r\n");
    }
    conn->preUser = conn->preRnfr = 0;
    write(conn->sock, msg, strlen(msg));
}

void rmd(char cmd[], Connection* conn){
    getPath(conn, dir, fullDir, cmd);
    char sysCmd[BUF_SIZE] = "rm -rf ";
    strcat(sysCmd, fullDir);
    if(!access(fullDir, F_OK) && !system(sysCmd)){
        strcpy(msg, "250 \"");
        strcat(msg, dir);
        strcat(msg, "\" directory removed.\r\n");
    } else {
        strcpy(msg, "550 The removal failed.\r\n");
    }
    conn->preUser = conn->preRnfr = 0;
    write(conn->sock, msg, strlen(msg));
}

void rnfr(char cmd[], Connection* conn){
    conn->preUser = 0;
    getPath(conn, dir, fullDir, cmd);
    if(!access(fullDir, F_OK)){
        strcpy(msg, "350 Please send the new pathname.\r\n");
        conn->preRnfr = 1;
        strcpy(conn->rnfrDir, fullDir);
    } else {
        strcpy(msg, "550 \"");
        strcat(msg, dir);
        strcat(msg, "\" doesn't exist.\r\n");
        conn->preRnfr = 0;
    }
    write(conn->sock, msg, strlen(msg));
}

void rnto(char cmd[], Connection* conn){
    if(conn->preRnfr){
        getPath(conn, dir, fullDir, cmd);
        if(!access(fullDir, F_OK)){
            strcpy(msg, "553 \"");
            strcat(msg, dir);
            strcat(msg, "\" already exists.\r\n");
        } else {
            rename(conn->rnfrDir, fullDir);
            strcpy(msg, "250 Renamed successfully.\r\n");
        }
    } else {
        strcpy(msg, "503 The previous request was not RNFR.\r\n");
    }
    conn->preRnfr = conn->preUser = 0;
    write(conn->sock, msg, strlen(msg));
}

void port(char cmd[], Connection* conn){
    conn->isPasv = 0;
    if(conn->auth == needTransferConn) close(conn->dataSock);
    setPortSocket(cmd, conn);
    conn->auth = needTransferConn;
    strcpy(msg, "200 Accepted.\r\n");
    write(conn->sock, msg, strlen(msg));
}

void pasv(char cmd[], Connection* conn){
    conn->isPasv = 1;
    if(conn->auth == needTransferConn) close(conn->dataSock);
    socklen_t addrSize = sizeof(struct sockaddr_in);
    struct sockaddr_in addr;
    int port = 20;
    getsockname(conn->sock, (struct sockaddr*)&addr, &addrSize);
    conn->dataSock = createSocket(INADDR_ANY, &port);
    conn->auth = needTransferConn;
    char pattern[] = "227 =%s,%d,%d\r\n";
    sprintf(msg, pattern, inet_ntoa(*((struct in_addr*)&(addr.sin_addr.s_addr))), port / 256, port % 256);
    char* p = strchr(msg, '.');
    while(p != NULL){
        *p = ',';
        p = strchr(p, '.');
    }
    write(conn->sock, msg, strlen(msg));
}

void list(char cmd[], Connection* conn){
    int flag = 1;
    conn->auth = normal;
    strcpy(msg, "150 Start transmitting the information of the file.\r\n");
    write(conn->sock, msg, strlen(msg));
    if(cmd[0] != ' ') cmd[0] = 0;
    getPath(conn, dir, fullDir, cmd);
    if(access(fullDir, F_OK) == -1){
        strcpy(msg, "451 The server had trouble reading the file from disk.\r\n");
    } else {
        int sock;
        if(getPortSocket(conn, &sock) < 0){
            strcpy(msg, "425 No TCP connection was established.\r\n");
        } else {
            struct stat fileStat;
            stat(fullDir, &fileStat);
            if(S_ISREG(fileStat.st_mode)){
                if(sendInfo(dir, &fileStat, sock) < 0){
                    strcpy(msg, "426 The TCP connection was established but then broken by the client or by network failure.\r\n");
                    flag = 0;
                }
            } else if(S_ISDIR(fileStat.st_mode)){
                DIR* dp = opendir(fullDir);
                struct dirent* fp;
                while((fp = readdir(dp)) != NULL && flag){
                    strcpy(name, fullDir);
                    strcat(name, "/");
                    strcat(name, fp->d_name);
                    stat(name, &fileStat);
                    if(sendInfo(fp->d_name, &fileStat, sock) < 0){
                        strcpy(msg, "426 The TCP connection was established but then broken by the client or by network failure.\r\n");
                        flag = 0;
                    }
                }
            }
            close(sock);
        }
        if(flag) strcpy(msg, "226 Transmitted successfully.\r\n");
    }
    close(conn->dataSock);
    write(conn->sock, msg, strlen(msg));
}

void retr(char cmd[], Connection* conn){
    getPath(conn, dir, fullDir, cmd);
    if(!access(fullDir, F_OK)){
        struct stat fileStat;
        stat(fullDir, &fileStat);
        if(S_ISREG(fileStat.st_mode)){
            strcpy(msg, "150 Start transferring the file.\r\n");
            write(conn->sock, msg, strlen(msg));
            conn->auth = normal;
            int sock;
            if(getPortSocket(conn, &sock) < 0){
                strcpy(msg, "425 No TCP connection was established.\r\n");
            } else {
                int fd = open(fullDir, O_RDONLY), ret;
                off_t offset = 0;
                while((ret = sendfile(sock, fd, &offset, fileStat.st_size)) > 0);
                if(ret < 0){
                    strcpy(msg, "426 The TCP connection was established but then broken by the client or by network failure.\r\n");
                } else {
                    strcpy(msg, "226 Transferred successfully.\r\n");
                }
                close(fd);
            }
            close(sock);
            close(conn->dataSock);
        }
    } else strcpy(msg, "451 The server had trouble reading the file from disk.\r\n");
    write(conn->sock, msg, strlen(msg));
}

void stor(char cmd[], Connection* conn){
    getPath(conn, dir, fullDir, cmd);
    FILE* fp = fopen(fullDir, "w");
    if(fp != NULL){
        strcpy(msg, "150 Start transferring the file.\r\n");
        write(conn->sock, msg, strlen(msg));
        conn->auth = normal;
        int fd = fileno(fp), sock, pair[2];
        const int SIZE = 8192;
        if(getPortSocket(conn, &sock) < 0){
            strcpy(msg, "425 No TCP connection was established.\r\n");
        } else {
            pipe(pair);
            long int offset = 0, ret;
            while((ret = splice(sock, NULL, pair[1], NULL, SIZE, 0)) > 0){
                splice(pair[0], NULL, fd, &offset, ret, 0);
            }
            if(ret < 0){
                printf("Error splice(): %s(%d)\n", strerror(errno), errno);
                strcpy(msg, "426 The TCP connection was established but then broken by the client or by network failure.\r\n");
            } else {
                strcpy(msg, "226 Transferred successfully.\r\n");
            }
        }
        close(fd);
        close(sock);
        close(conn->dataSock);
    } else strcpy(msg, "451 The server had trouble reading the file from disk.\r\n");
    write(conn->sock, msg, strlen(msg));
}
