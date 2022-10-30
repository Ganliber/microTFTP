#include "data.h"
#include "wrap.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <string.h> // memset

int hint();
int upload_file(char * cli_file);
int download_file(char * serv_file);

/* Global Variables */
struct sockaddr_in servaddr; // server info struct
int sockfd, mode;   // socket fd for datagram transmission
                    // mode=1 netascii; mode=2 octet(default)
socklen_t servaddr_len;

/* About UDP & TCP*/
// send() & recv() --> TCP
// sendto() & recvfrom() --> UDP
/*
* Definition:
    ssize_t sendto(int sockfd, const void *buf, size_t len, int flags,
                const struct sockaddr *dest_addr, socklen_t addrlen);
* For simple : 
    sendto(sockfd, buf, len, flags, NULL, 0);
*/
int main(int argc, char* argv[]) {

    char command[MAXLINE]; // Input command
    mode = 1; // default value : octet (binary)

    if(argc < 2) {
        printf("Usage: %s ip [port]\n",argv[0]);
        printf("Micro tftp client to upload or download trivial file to server.\n");
        printf("The default port is 69.\n");
        return 0;
    }
  

    sockfd = Socket(AF_INET, SOCK_DGRAM, 0); // fd recieved from socket, Socket contains error handler

    //test
    printf("The connecting ip addr is: %s\n", argv[1]);
    
    bzero(&servaddr, sizeof(servaddr)); 
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(PORT);
    inet_pton(AF_INET, argv[1], &servaddr.sin_addr);


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
                // printf("%s\n", ins);
                download_file(ins);
            }
        } else if (strcmp(ins, "put")==0 || strcmp(ins, "upload")==0) {
            ins = strtok(NULL, " \t\\n");
            if(ins == NULL) {
                printf("\n\033[31m**[error : missing remote file name.]**\033[0m\n\n");
            } else {
                upload_file(ins);
            }
        } else if (strcmp(ins, "shell")==0) {
            /*  
            *   support for shell instructions
            *   shell ls 
            *   shell pwd
            *   shell clear
            *   shell echo
            *   shell uname -r
            *   ...
            */ 
            ins = strtok(NULL, " \t\n");
            char shell[MAXLINE]; // shell instruction length
            int index = 0;
            while(ins!=NULL) {
                // exec shell instructions
                while(*ins!='\0'){
                    shell[index] = *ins;
                    index++;
                    ins++;
                }
                shell[index] = ' ';
                index++;
                ins = strtok(NULL, " \t\n");
            }
            shell[index] = '\0';
            system(shell);
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
/// @return err_code
int download_file(char * serv_file) {
    tftp_dgram sendpkt, ackpkt, recvpkt;

    // error
    if(serv_file==NULL){
        return -1;
    }

    /* RRQ Datagram */
    sendpkt.opcode = RRQ;
    sendpkt.datagram.request.opcode = htons(RRQ);

    int i=0;
    while (serv_file[i]!='\0') {
        sendpkt.datagram.request.filename_and_mode[i] = serv_file[i];//?
        i ++;
    }
    sendpkt.datagram.request.filename_and_mode[i++] = '\0';
    char *modestr;

    if(mode==1) modestr = "NETASCII";
    else modestr = "OCTET";

    while (*modestr!='\0') {
        sendpkt.datagram.request.filename_and_mode[i++] = *modestr;
        modestr++;
    }
    sendpkt.datagram.request.filename_and_mode[i] = '\0';

    sendto(sockfd, &sendpkt.datagram, sizeof(sendpkt.datagram),
                    0, (struct sockaddr *)&servaddr, servaddr_len);
    
    
    /* recvfrom:
     * return the length of the message on successful completion.
     */
    int recvbytes;
    uint16_t tmp_opcode;
    uint16_t tmp_blocknum;

    FILE *file_download;
    file_download = fopen(serv_file, "r"); // check if it exists already, if so, cover it.
    if (file_download == NULL) {
        // it doesn't exist, so u need to create it.
        file_download = fopen(serv_file, "w");
    }



    while(1) {
        recvbytes = recvfrom(sockfd, &recvpkt.datagram, sizeof(recvpkt.datagram),
                        MSG_DONTWAIT, (struct sockaddr *)&servaddr, &servaddr_len);

        if (recvbytes > 0 && recvbytes < 4) {
            printf("Bad packet recieved.\n");
            continue;
        } else {
            tmp_opcode = recvpkt.datagram.data.opcode;
            tmp_blocknum = recvpkt.datagram.data.blocknum;
            if(tmp_opcode != DATA) {
                printf("Not proper data packet.\n");
                continue;
            }       
            if (recvbytes == DGRAM_SIZE) {
                /* intermediate packets */
                printf("Recieved => block# %d, size=%d ...\n", tmp_blocknum, recvbytes - 4);

            } else if (recvbytes >= 4 && recvbytes < DGRAM_SIZE) {
                /* Last packet */
                printf("block# %d has been recieved... And it is the last block.\n", tmp_blocknum);
            }

        } 

        ackpkt.opcode = ACK;
        ackpkt.datagram.ack.opcode = htons(ACK);
        ackpkt.datagram.ack.blocknum = tmp_blocknum;
    }
    return 1;
}