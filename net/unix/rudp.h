/******************************************
/*          RUDP HEADER
/*****************************************/
#include <sys/socket.h>
#include <arpa/inet.h>


ssize_t rudp_recvfrom(int __fd, void *__restrict __buf, size_t __n, int __flags,
__SOCKADDR_ARG __addr, socklen_t *__restrict __addr_len);

ssize_t rudp_sendto (int __fd, const void *__buf, size_t __n,int __flags,
__CONST_SOCKADDR_ARG __addr,socklen_t __addr_len);