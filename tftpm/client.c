#include "client.h"
#include <sys/time.h>

/* Global Variables */
struct sockaddr_in servaddr; // server info struct
int sockfd, global_mode;    // socket fd for datagram transmission
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
    global_mode = 1; // default value : octet (binary)

    if(argc < 2) {
        printf("Usage: %s ip [port]\n",argv[0]);
        printf("Micro tftp client to upload or download trivial file to server.\n");
        printf("The default port is 69.\n");
        return 0;
    }

    // Timeout handler 
    //struct timeval timeout = {TIME_OUT,0}; // 15s timeout
    sockfd = Socket(AF_INET, SOCK_DGRAM, 0); // fd recieved from socket, Socket contains error handler

    /*
    * Before sendto
    *   struct timeval timeout = {TIME_OUT,0}; // 15s timeout
    *   int ret=setsockopt(sockfd,SOL_SOCKET,SO_SNDTIMEO,(const char*)&timeout,sizeof(timeout));
    * Before recvfrom
    *   struct timeval timeout = {TIME_OUT,0}; // 15s timeout
    *   int ret=setsockopt(sockfd,SOL_SOCKET,SO_RCVTIMEO,(const char*)&timeout,sizeof(timeout));
    */

    //test
    printf("The connecting ip addr is: %s\n", argv[1]);
    
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    inet_pton(AF_INET, argv[1], &servaddr.sin_addr);
    servaddr.sin_port = htons(PORT);
    servaddr_len = sizeof(servaddr);

    printf("Initialization finished ... \n");

    char *buffer, *ins;// ins is instructions
    while (1)
    {
        //printf(" \033[36mM\033[34mi\033[36mc\033[34mr\033[36mo\033[34mT\033[36mF\033[34mT\033[36mP\033[0m\033[35m\033[1m ->\033[33m ");
        printf("\033[36mMicro\033[34mTFTP\033[0m\033[35m\033[1m ->\033[33m ");
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
            Close(sockfd);
            exit(0);printf("%s\n", ins);
        } else if (strcmp(ins, "help")==0 || strcmp(ins, "h")==0) {
            hint();
        } else if (strcmp(ins, "get")==0 || strcmp(ins, "download")==0) {
            ins = strtok(NULL, " \t\n"); // continue get the following sub-string
            if(ins == NULL) {
                printf("\n\033[31m**[error : missing remote file name.]**\033[0m\n\n");
            } else {
                // printf("%s\n", ins);
                download_file(ins,argv[1]);
                exit(0);
            }
        } else if (strcmp(ins, "put")==0 || strcmp(ins, "upload")==0) {
            ins = strtok(NULL, " \t\n");
            if(ins == NULL) {
                printf("\n\033[31m**[error : missing remote file name.]**\033[0m\n\n");
            } else {
                upload_file(ins, argv[1]);
                exit(0);
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
        } else if (strcmp(ins, "checkmode")==0) {
            char * modestr;
            if(global_mode == 1) 
                modestr = "NETASCII";
            else
                modestr = "OCTET";
            printf("The current mode to transfer files is \033[31m%s\033[33m\n", modestr);
        } else if (strcmp(ins, "setmode")==0) {
            ins = strtok(NULL, " \t\n");
            char * modestr;
            if (strcmp(ins, "1")==0) {
                global_mode = 1;
                modestr = "NETASCII";
            } else if (strcmp(ins, "2")==0) {
                global_mode = 2;
                modestr = "OCTET";
            } else {
                printf("setmode [1|2]\t\t --- NETASCII: mode=1 OCTET: mode=2\n");
                continue;
            }
            printf("The current mode to transfer files has been changed to \033[31m%s\033[33m\n", modestr);
        } else {
            printf("Invalid command, please input \"help\" to check.\n");
        }
    }
    return 0;
}

/// @brief 
/// @return 
int hint() {
    printf("usage: \n\n");
    printf("help\t\t\t\t --- check out the help guide.\n\n");
    printf("checkmode\t\t\t --- check the transmission mode.\n\n");
    printf("setmode [1|2]\t\t\t --- set the value of transmission mode.\n \t\t\t\t     NETASCII:\tmode = 1\n\t\t\t\t     OOCTET:\tmode = 2\n\n");
    printf("put | upload [local file]\t --- upload local file to remote server.\n\n");
    printf("get | download [remote file]\t --- download file from remote server.\n\n");
    printf("shell [shell instruction]\t --- execute shell instructions.\n\n");
    printf("quilt | q\t\t\t --- quilt connection to TFTP server.\n\n");
}

/// @brief 
/// @param clifile 
/// @return success code: 1 for success & 0 for failure 
int upload_file(char *cli_file, char *server_ip) {
    //reload_socket();
    tftp_dgram sendpkt, ackpkt, recvpkt;
    int size = sizeof(sendpkt);

    if(cli_file == NULL) {
        return -1;
    }

    sendpkt.opcode = htons(WRQ); // write request

    int i = 0;
    while(cli_file[i]!='\0') {
        sendpkt.datagram.request.filename_and_mode[i] = cli_file[i];
        i ++;
    }
    sendpkt.datagram.request.filename_and_mode[i++] = '\0';
    char * modestr;
    int tmp = i;
    if(global_mode == 1) modestr = "netascii";
    else modestr = "octet";

    while(*modestr!='\0') {
        sendpkt.datagram.request.filename_and_mode[i++] = *modestr;
        modestr ++;
    }
    sendpkt.datagram.request.filename_and_mode[i] = '\0';

    ssize_t res = sendto(sockfd, &sendpkt, size, 0, (struct sockaddr *)&servaddr, servaddr_len);

    if(res == -1) {
        
        perr_exit("Sending tftp datagram failed!\n");
    }

    /*
    * recvfrom : ACK block#0 datagram
    */
    int recvbytes;
    uint16_t tmp_opcode;
    uint16_t tmp_blocknum;
    uint16_t cur_blocknum = 0;
    int last_blocknum = -1;

    FILE *fp;
    if(global_mode == 1) {
        fp = fopen(cli_file,"r");
    } else if (global_mode == 2) {
        fp = fopen(cli_file, "rb");
    }

    int timeout_number = 0;// if time of timeout is above 3, exit.
    while(1) {
        struct timeval timeout = {TIME_OUT,0}; // 15s timeout
        int ret=setsockopt(sockfd,SOL_SOCKET,SO_RCVTIMEO,(const char*)&timeout,sizeof(timeout));
        recvbytes = recvfrom(sockfd, &recvpkt, sizeof(recvpkt), 0, 
                        (struct sockaddr *)&servaddr, &servaddr_len);

        if(recvbytes == -1 && errno == EAGAIN) {
            timeout_number += 1;
            if (timeout_number > 3) {
                perr_exit("\n\033[31mTimeout for third time. Sorry we have to terminate this transmission, please wait for a while and try again.\n\033[33m\n");
            }
            printf("\n\033[31mTimeout. Try to upload again ...\n\n\033[33m");
            ssize_t res = sendto(sockfd, &sendpkt, size, 0, (struct sockaddr *)&servaddr, servaddr_len);
            if(res == -1) {
                perr_exit("Sending tftp datagram failed!\n");
            }
            continue;
        }

        if (recvbytes < 4) {
            perr_exit("Bad packet recieved.\n");
        }
        tmp_opcode = ntohs(recvpkt.opcode);
        if(tmp_opcode == ACK) {
            tmp_blocknum = ntohs(recvpkt.datagram.ack.blocknum);
            //printf("last num = %d; tmp = %d\n", last_blocknum, tmp_blocknum);
            
            if (last_blocknum == (int)tmp_blocknum) {
                printf("block \033[31m#%d\033[33m \033[34m(Last block)\033[33m acceptance confirmed => $ v $\n\n", cur_blocknum);
                break;
            } else if(tmp_blocknum == 0 && cur_blocknum == 0) {
                printf("WRQ accepted ... \n\n### Uploading \033[33mto \033[37m%s \033[33m###\n\n", server_ip);
            } else if (tmp_blocknum != 0 && tmp_blocknum == cur_blocknum) {
                printf("block \033[31m#%d\033[33m acceptance confirmed => ^_^\n\n", cur_blocknum); 
            } else {
                printf("%d\n",tmp_blocknum);
                //printf("\n\033[31m**[ ACK ERROR ]**\033[0m\n\n");
                perr_exit("\n\033[31m**[ ACK ERROR ]**\033[0m\n\n"); // EXIT !!!
                //continue;
            }
            /* Send block */
            cur_blocknum += 1;
            ssize_t err_code;
            memset(&sendpkt, '\0', sizeof(sendpkt)); // important
            // sending block
            sendpkt.opcode = htons(DATA);
            sendpkt.datagram.data.blocknum = htons(cur_blocknum);
            err_code = fread(&sendpkt.datagram.data.data, 1, MAXLINE, fp);
            //printf("*** fread %ld bytes ***\n", err_code);

            if (err_code < MAXLINE) {
                printf("sending last block (size=%ld) \033[31m#%d\033[33m ...\n", err_code,cur_blocknum);
                //printf("WTF ???\n");
                last_blocknum = cur_blocknum;
                if (err_code == 0)
                    size = 4;
                else
                    size = 4 + err_code;
            } else {
                // res == MAXLINE
                printf("sending block \033[31m#%d\033[33m ...\n", cur_blocknum);
                //sendto(sockfd, &sendpkt, sizeof(sendpkt), 0, (struct sockaddr *)&servaddr, servaddr_len);
            }
            sendto(sockfd, &sendpkt, size, 0, (struct sockaddr *)&servaddr, servaddr_len);
        } else {
            printf("\n\033[31m**[ ACK ERROR: datagram accepted type is %d. ]**\033[0m\n\n", tmp_opcode);
        }
    }
    printf("\n###\033[32m File Upload Success\033[33m ###\n\n");
    fclose(fp);
    return 1;//succeed
}

/// @brief 
/// @param serv_file 
/// @param cli_file 
/// @return err_code
int download_file(char * serv_file, char* server_ip) {
    tftp_dgram sendpkt, ackpkt, recvpkt;
    
    //printf("The size of tftp packet is: %lu\n",sizeof(sendpkt));
    // error
    if(serv_file==NULL){
        return -1;
    }

    /* RRQ Datagram */
    sendpkt.opcode = htons(RRQ);

    int i=0;
    while (serv_file[i]!='\0') {
        sendpkt.datagram.request.filename_and_mode[i] = serv_file[i];//?
        i++;
    }
    sendpkt.datagram.request.filename_and_mode[i++] = '\0';
    char *modestr;
    int tmp = i;

    if(global_mode==1) modestr = "netascii";
    else modestr = "octet";

    while (*modestr!='\0') {
        sendpkt.datagram.request.filename_and_mode[i++] = *modestr;
        modestr++;
    }
    sendpkt.datagram.request.filename_and_mode[i] = '\0';

    ssize_t res = sendto(sockfd, &sendpkt, sizeof(sendpkt),
                    0, (struct sockaddr *)&servaddr, servaddr_len);

    if(res == -1) {
        perr_exit("Sending tftp datagram failed!\n");
    }
    /* 
     * recvfrom:
     * return the length of the message on successful completion.
     */
    int recvbytes;
    uint16_t tmp_opcode;
    uint16_t tmp_blocknum;
    uint16_t cur_blocknum = 1;
    ssize_t err_code;

    FILE *file_download;
    if(global_mode == 1)
        file_download = fopen(serv_file, "w"); // text format and appendable
    else if(global_mode == 2)
        file_download = fopen(serv_file, "wb"); // binary format and appendable

    printf("\n### Downloading \033[33mfrom \033[37m%s \033[33m###\n\n", server_ip);
    
    int timeout_number = 0;// if time of timeout is above 3, exit.
    int ack_size = 4;
    
    while(1) {
        struct timeval timeout = {TIME_OUT,0}; // 15s timeout
        int ret=setsockopt(sockfd,SOL_SOCKET,SO_RCVTIMEO,(const char*)&timeout,sizeof(timeout));
        recvbytes = recvfrom(sockfd, &recvpkt, sizeof(recvpkt), 0, 
                        (struct sockaddr *)&servaddr, &servaddr_len);

        if(recvbytes == -1 && errno == EAGAIN) {
            timeout_number += 1;
            if (timeout_number >= 3) {
                perr_exit("\n\033[31mTimeout for third time. Sorry we have to terminate this transmission, please wait for a while and try again.\n\033[33m\n");
            }
            printf("\n\033[31mTimeout. Try to upload again ...\n\n\033[33m");
            sendto(sockfd, &ackpkt, ack_size, 0, (struct sockaddr*)&servaddr, servaddr_len);
            continue;
        }

        if (recvbytes < 4) {
            perr_exit("Bad packet recieved.\n");
        } else {
            tmp_opcode = ntohs(recvpkt.opcode);
            
            if(tmp_opcode != DATA) {
                printf("Not proper DATA packet => pkt type is: %d\n", tmp_opcode);
                if (tmp_opcode == ERROR) {
                    // This time the error message is stored in the recvpkt
                    uint16_t err_code = ntohs(recvpkt.datagram.error.err_code);
                    printf("ERROR CODE:\n\t%d\n", err_code);
                    printf("ERROR MESSAGE:\n\t%s\n", recvpkt.datagram.error.err_msg);
                    fclose(file_download);
                    perr_exit("\n\033[31m**[ ERROR DATAGRAM RECIEVED ]**\033[0m\n\n");
                }
                continue;
                //perr_exit("Not proper data packet.\n");
            }      

            tmp_blocknum = ntohs(recvpkt.datagram.data.blocknum);
            if (tmp_blocknum == cur_blocknum && tmp_opcode == DATA) {
                /* intermediate packets */
                //printf("Block #%d recieved => size = %d bytes ...", tmp_blocknum, recvbytes - 4);                
                err_code = fwrite(&recvpkt.datagram.data.data,1,recvbytes - 4, file_download);
                
                //printf("*** fwrite %ld bytes ***\n", err_code);
                
                if (err_code != recvbytes - 4) {
                    printf("ERROR: fwrite triggered an writing error!\n");
                    fclose(file_download);
                    perr_exit(0);
                }

                if (recvbytes < DGRAM_SIZE) {
                    printf("Block \033[31m#%d\033[33m \033[34m(Last block)\033[33m recieved => size = %d bytes ...\n", tmp_blocknum, recvbytes - 4);
                    ackpkt.opcode = htons(ACK);
                    ackpkt.datagram.ack.blocknum = htons(tmp_blocknum);
                    //ACK
                    sendto(sockfd, &ackpkt, ack_size, 0, (struct sockaddr*)&servaddr, servaddr_len);           
                    break;
                } else {
                    printf("Block \033[31m#%d\033[33m recieved => size = %d bytes ...\n", tmp_blocknum, recvbytes - 4);
                    
                    printf("\n");
                    ackpkt.opcode = htons(ACK);
                    ackpkt.datagram.ack.blocknum = htons(tmp_blocknum);
                    //ACK
                    sendto(sockfd, &ackpkt, ack_size, 0, (struct sockaddr*)&servaddr, servaddr_len);             
                    cur_blocknum += 1;
                    continue;
                }
            }
        }  
    }
    printf("\n###\033[32m File Download Success\033[33m ###\n\n");
    fclose(file_download);
    return 1;
}