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
int upload_file(char * cli_file, char * server_ip);
int download_file(char * serv_file, char * server_ip);
