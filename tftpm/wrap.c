// /* Wrap some new functions & Add some error handlers */
// #include <stdio.h>
// #include <stdlib.h>
// #include <errno.h>
// #include <sys/socket.h>
// #include <sys/types.h> // where ssize_t is defined
// #include <unistd.h>
#include "wrap.h"


/// @brief wrapped error printer & exit function
/// @param s 
void perr_exit(const char*s) {
    perror(s);
    exit(1);
}


/// @brief wrapped 'accept' with error handler
/// @param fd 
/// @param sa 
/// @param salenptr 
/// @return 
int Accept(int fd, struct sockaddr *sa, socklen_t *salenptr) {
    int n;
again:
    if ((n = accept(fd, sa, salenptr)) < 0) {
        if ((errno == ECONNABORTED) || (errno == EINTR)) {
            goto again;
        } 
        else {
            perr_exit("accept error");
        }
    }
    return n;
}


/// @brief wrapped 'bind' with error handler
/// @param fd 
/// @param sa 
/// @param salen 
void Bind( int fd, const struct sockaddr *sa, socklen_t salen) {
    if (bind(fd, sa, salen) < 0) {
        perr_exit("bind error");
    }
}


/// @brief wrapped 'connect' with error handler
/// @param fd 
/// @param sa 
/// @param salen 
void Connect(int fd, const struct sockaddr *sa, socklen_t salen) {
    if (connect(fd, sa, salen) < 0) {
        perr_exit("connect error");
    }
}


/// @brief wrapped 'listen' with error handler
/// @param fd 
/// @param backlog 
void Listen(int fd, int backlog) {
    if (listen(fd, backlog) < 0) {
        perr_exit("listen error");
    }
}


/// @brief wrapped 'socket' with error handler
/// @param family 
/// @param type 
/// @param protocol 
int Socket(int family, int type, int protocol) {
    int n;
    if( (n = socket(family, type, protocol)) < 0) {
        perr_exit("socket error");
    }
    return n;
}


/// @brief wrapped 'close' with error handler
/// @param fd 
void Close(int fd) {
    if (close(fd) == -1) {
        perr_exit("close error");
    }
}


/// @brief wrapped 'write' with error handler
/// @param fd 
/// @param ptr 
/// @param nbytes 
/// @return 
ssize_t Write(int fd, const void* ptr, size_t nbytes) {
    ssize_t n;
again:
    if ( (n = write(fd, ptr, nbytes)) < 0 ) {
        if (errno == EINTR) {
            goto again;
        }
        else {
            return -1;
        }
    }
    return n;
}


/// @brief wrapped 'read' with error handler
/// @param fd 
/// @param ptr 
/// @param nbytes 
/// @return 
ssize_t Read(int fd, void * ptr, size_t nbytes) {
    ssize_t n;
again:
    if ( (n = read(fd, ptr, nbytes)) == -1) {
        if (errno == EINTR) {
            goto again;
        } 
        else {
            return -1;
        }
    }
    return n;
}


/// @brief wrapped 'read' with error handler
/// @param fd 
/// @param vptr 
/// @param nbytes 
/// @return 
ssize_t Readn(int fd, void *vptr, size_t nbytes) {
    size_t nleft;
    ssize_t nread;
    char *ptr;
    
    ptr = vptr; // 读到的位置, 不能更改初始位置的指针, 所以需要该临时变量
    nleft = nbytes;
    while (nleft > 0) {// 表示持续地读, 如果一次没有读完请求的字节数, 这样可以支持异步
        if ( (nread = read(fd, ptr, nleft)) < 0) {
            if (errno == EINTR)
                nread = 0;
            else
                return -1;
        } else if ( nread == 0) {
            // EOF
            break;
        }

        nleft -= nread;//和请求量比起来还有所少没有读完
        ptr += nread;
    }

    return nbytes - nleft;//读了的字节数
}


/// @brief wrap 'write' with error handler
/// @param fd 
/// @param vptr 
/// @param nbytes 
/// @return 
ssize_t Writen(int fd, const void* vptr, size_t nbytes) {
    size_t nleft;
    ssize_t nwritten;
    const char *ptr;

    ptr = vptr;
    nleft = nbytes;
    while(nleft > 0) {
        if ((nwritten = write(fd, ptr, nleft)) <= 0) {
            if (nwritten < 0 & errno == EINTR) {
                nwritten = 0;
            } else {
                return -1;
            }
        }

        nleft -= nwritten;
        ptr += nwritten;
    }
    return nbytes;
}


static ssize_t my_read(int fd, char *ptr) {
    static int read_cnt;
    static char *read_ptr;
    static char read_buf[100];

    if (read_cnt <= 0) {
    again:
        if ( (read_cnt = read(fd, read_buf, sizeof(read_buf))) < 0 ) {
            if (errno == EINTR) {
                goto again;
            } else if (read_cnt == 0) {
                return 0;
            }
        }
        
    }
    read_cnt--;
    *ptr = *read_ptr++;
    return 1;
}

ssize_t Readline(int fd, void *vptr, size_t maxlen) {
    ssize_t n,rc;
    char c, *ptr;

    ptr = vptr;
    for (n = 1; n < maxlen; n++) {
        if((rc = my_read(fd, &c)) == 1) {
            *ptr++ = c;
            if(c=='\n')
                break;
        } else if (rc==0) {
            *ptr = 0;
            return n-1;
        } else {
            return -1;
        }
    }
    *ptr = 0;
    return n;
}