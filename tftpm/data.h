#include "wrap.h"
#include <stdint.h>

#define PORT 69
#define BLOCK_LEN 512
#define MAXLINE 512 // longest instrument in CLI

/* tftp opcode mnemonic */
enum opcode {
     RRQ=1, WRQ, DATA, ACK, ERROR
};

/* tftp transfer mode */
enum mode {
     NETASCII=1, // netASCII -> text file
     OCTET // net datagram in binary
};

/* tftp message structure */
typedef struct {

    uint16_t opcode;// decide which kind of datagram is 

    union {
        struct {
            uint16_t opcode; /* RRQ(1) or WRQ(2) */             
            uint8_t filename_and_mode[514]; // (longest) 512 bytes + 2 bytes for '\0'
        } request;     

        struct {
            uint16_t opcode; /* DATA */
            uint16_t blocknum; // block#
            uint8_t data[BLOCK_LEN];
        } data;

        struct {
            uint16_t opcode; /* ACK */             
            uint16_t blocknum; // block# 2 bytes
        } ack;

        struct {
            uint16_t opcode; /* ERROR */     
            uint16_t err_code;
            uint8_t err_msg[BLOCK_LEN];
        } error;
    } datagram;

} tftp_dgram;