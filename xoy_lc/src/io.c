#include "io.h"

#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>

// io connect
static int io_connect(char *ip, short port, int family, int type);

int io_tcp_connect(char *ip, short port) {
  return io_connect(ip, port, AF_INET, SOCK_STREAM);
}
int io_udp_connect(char *ip, short port) {
  return io_connect(ip, port, AF_INET, SOCK_DGRAM);
}

// io connect
static int io_connect(char *ip, short port, int family, int type) {
  int sockfd;
  struct sockaddr_in servaddr;

  bzero(&servaddr, sizeof(servaddr));
  servaddr.sin_family = family;
  servaddr.sin_addr.s_addr = inet_addr(ip);
  servaddr.sin_port = htons(port);

  if ((sockfd = socket(family, type, 0)) < 0) {
    fprintf(stderr, "socket error\n");
    return -1;
  }

  if (connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
    fprintf(stderr, "connect ip<%s> error\n", ip);
    return -1;
  }

  return sockfd;
}

// io reand and write line
ssize_t io_readn(int fd, void *vptr, size_t n) {
  size_t nleft;
  ssize_t nread;
  char *ptr;

  ptr = vptr;
  nleft = n;

  while(nleft > 0) {
    if((nread = read(fd, ptr, nleft)) < 0) {
      if(errno == EINTR)
        nread = 0; /* and call read() again */
      else
        return -1;
    }else if(nread == 0)
      break; /* EOF */

    nleft -= nread;
    ptr += nread;
  }

  return n - nleft; /* return >= 0 */
}

ssize_t io_writen(int fd, const void *vptr, size_t n) {
  size_t nleft;
  ssize_t nwritten;
  const char *ptr;

  ptr = vptr;
  nleft = n;
  while(nleft > 0) {
    if((nwritten = write(fd, ptr, nleft)) < 0) {
      if(nwritten < 0 && errno == EINTR)
        nwritten = 0; /* and call write() again */
      else
        return -1;
    }

    nleft -= nwritten;
    ptr += nwritten;
  }

  return n;
}

ssize_t io_readline(int fd, void *vptr, size_t maxlen) {
  ssize_t n, rc;
  char c, *ptr;

  ptr = vptr;
  for(n = 1; n < maxlen; n++) {
    again:
      if((rc = read(fd, &c, 1)) == 1) {
        *ptr++ = c;
        if(c == '\n')
          break;  /* newline is stored, like fgets() */
      } else if (rc == 0) {
        *ptr = 0;
        return n - 1; /* EOF, n -1 bytes were read */
      } else {
        if (errno == EINTR)
          goto again;

        return -1;  /* error, errno set by read() */
      }
  }

  *ptr = 0;
  return n;
}


