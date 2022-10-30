#include "wrap.h"
#include <stdint.h>

#define PORT 69
#define BLOCK_LEN 512
#define MAXLINE 512 // longest instrument in CLI
#define DGRAM_SIZE 516

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
typedef struct TFTP_DGRAM{

    uint16_t opcode; // decide which kind of datagram is 
                     // 2 bytes

    union {

        struct {
            uint16_t opcode; /* RRQ(1) or WRQ(2) */             
            uint8_t filename_and_mode[514]; // (longest) 512 bytes + 2 bytes for '\0'
        } request; // 516 bytes     

        struct {
            uint16_t opcode; /* DATA */
            uint16_t blocknum; // block#
            uint8_t data[BLOCK_LEN];
        } data; // 516 bytes

        struct {
            uint16_t opcode; /* ACK */             
            uint16_t blocknum; // block# 2 bytes
        } ack; // 4 bytes

        struct {
            uint16_t opcode; /* ERROR */     
            uint16_t err_code;
            uint8_t err_msg[BLOCK_LEN];
        } error; // 516 bytes

    } datagram;// 516 bytes

} tftp_dgram; // 518 bytes