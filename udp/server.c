/* udp server */
#include "wrap.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <ctype.h> 
    // toupper() & tolower()

#define MAXLINE 80
#define SERV_PORT 8000

int main() {
    struct sockaddr_in servaddr, cliaddr;
    socklen_t cliaddr_len;
    int sockfd;
    char buf[MAXLINE];
    char str[INET_ADDRSTRLEN];

    int i, n;
    sockfd = Socket(AF_INET, SOCK_DGRAM, 0); // Initialize a socket 

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(SERV_PORT);

    Bind(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr));

    printf("Accepting connections ... \n");

    while(1) {
        cliaddr_len = sizeof(cliaddr);
        n = recvfrom(sockfd, buf, MAXLINE, 0, (struct sockaddr *)&cliaddr, &cliaddr_len);
        if(n == -1) {
            perr_exit("recvfrom error!\n");
        }
        printf("Recieve from %s at PORT %d\n", inet_ntop(AF_INET, &cliaddr.sin_addr,str, sizeof(str)), ntohs(cliaddr.sin_port));

/* Server function */
        for (i = 0; i<n; i++) {
            buf[i] = toupper(buf[i]);
        }
/* Server function */

        n = sendto(sockfd, buf, n, 0, (struct sockaddr *)&cliaddr, sizeof(cliaddr));
        if(n==-1) {
            perr_exit("sendto error\n");
        }
    }

    return 0;
}