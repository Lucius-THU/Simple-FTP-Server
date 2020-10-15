#ifndef CMD_H
#define CMD_H
#include "basic.h"
#define CMD_CNT 17

typedef struct Cmd{
    Auth auth;
    char prefix[6];
    int len;
    void (*func)(char*, Connection*);
} Cmd;

extern const Cmd cmds[CMD_CNT];

void user(char cmd[], Connection* conn);
void pass(char cmd[], Connection* conn);
void syst(char cmd[], Connection* conn);
void type(char cmd[], Connection* conn);
void cwd(char cmd[], Connection* conn);
void pwd(char cmd[], Connection* conn);
void mkd(char cmd[], Connection* conn);
void rmd(char cmd[], Connection* conn);
void rnfr(char cmd[], Connection* conn);
void rnto(char cmd[], Connection* conn);
void port(char cmd[], Connection* conn);
void pasv(char cmd[], Connection* conn);
void list(char cmd[], Connection* conn);
void retr(char cmd[], Connection* conn);
void stor(char cmd[], Connection* conn);
void quit(char cmd[], Connection* conn);
void rest(char cmd[], Connection* conn);

#endif
