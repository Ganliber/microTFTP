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
#include <sys/time.h> // timeval struct for check if it is timeout

int hint();
int upload_file(char * cli_file, char * server_ip);
int download_file(char * serv_file, char * server_ip);