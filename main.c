#include "server.h"
#include <stdlib.h>
#include <string.h>

int main(int argc, char* argv[]){
    char root[BUF_SIZE] = "/tmp";
    int port = 21;
    for(int i = 1; i < argc; i += 2){
        if(!strcmp(argv[i], "-port")){
            port = atoi(argv[i + 1]);
        } else if(!strcmp(argv[i], "-root")){
            strcpy(root, argv[i + 1]);
        }
    }

    initServer(port, root);
    return 0;
}
