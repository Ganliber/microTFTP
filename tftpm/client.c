#include "data.h"
#include "wrap.h"

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <string.h> // memset

int hint();
int upload_file(char * cli_file);
int download_file(char * serv_file);


int main(int argc, char* argv[]) {

    char command[MAXLINE];

    if(argc < 2) {
        printf("Usage: %s ip [port]\n",argv[0]);
        printf("Micro tftp client to upload or download trivial file to server.\n");
        printf("The default port is 69.\n");
        return 0;
    }

    char *buffer, *ins;// ins is instructions
    while (1)
    {
        //printf(" \033[36mM\033[34mi\033[36mc\033[34mr\033[36mo\033[34mT\033[36mF\033[34mT\033[36mP\033[0m\033[35m\033[1m ->\033[33m ");
        printf(" \033[36mMicro\033[34mTFTP\033[0m\033[35m\033[1m ->\033[33m ");
        memset(command, 0, MAXLINE);
        buffer = fgets(command, MAXLINE, stdin);
        if(buffer==NULL) {
            printf("\n --- See u again --- \n");
            return 0;
        }

        ins = strtok(buffer, " \t\n");

        //printf("%s\n", ins);

        if (ins == NULL) {
            continue;
        } else if (strcmp(ins, "quit")==0 || strcmp(ins, "q")==0) {
            exit(0);printf("%s\n", ins);
        } else if (strcmp(ins, "help")==0 || strcmp(ins, "h")==0) {
            hint();
        } else if (strcmp(ins, "get")==0 || strcmp(ins, "download")==0) {
            ins = strtok(NULL, " \t\n"); // continue get the following sub-string
            if(ins == NULL) {
                printf("\n\033[31m**[error : missing remote file name.]**\033[0m\n\n");
            } else {
                //printf("%s\n", ins);
                download_file(ins);
            }
        } else if (strcmp(ins, "put")==0 || strcmp(ins, "upload")==0) {
            ins = strtok(NULL, " \t\\n");
            if(ins == NULL) {
                printf("\n\033[31m**[error : missing remote file name.]**\033[0m\n\n");
            } else {
                upload_file(ins);
            }
        } else if (strcmp(ins, "get")==0 || strcmp(ins, "download")==0) {

        }
    }
    return 0;
}

/// @brief 
/// @return 
int hint() {
    printf("usage: ");
}

/// @brief 
/// @param clifile 
/// @return success code: 1 for success & 0 for failure 
int upload_file(char *cli_file) {
    
    return 1;//succeed
}

/// @brief 
/// @param serv_file 
/// @param cli_file 
/// @return 
int download_file(char * serv_file) {

    return 1;
}