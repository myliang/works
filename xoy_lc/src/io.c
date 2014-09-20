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
#include <netdb.h>
#include <errno.h>

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
  struct addrinfo hints, *res;

  bzero(&hints, sizeof(struct addrinfo));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = type;

  char port_str[20];
  bzero(port_str, 20);
  sprintf(port_str, "%d", port);

  if (getaddrinfo(ip, port_str, &hints, &res) != 0) {
    fprintf(stderr, "%s:%d getaddrinfo<%s, %d> error: %s\n", __FILE__, __LINE__, ip, port, strerror(errno));
    return -1;
  }

  do {
    printf("protocol: %d\n", res->ai_protocol);
    if ((sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) < 0) {
      fprintf(stderr, "%s:%d socket error: %s\n", __FILE__, __LINE__, strerror(errno));
      continue;
    }

    if (connect(sockfd, res->ai_addr, res->ai_addrlen) < 0) {
      fprintf(stderr, "%s:%d connect ip<%s> error: %s\n", __FILE__, __LINE__, ip, strerror(errno));
    } else break;

    if (close(sockfd) < 0) {
      fprintf(stderr, "%s:%d close error: %s\n", __FILE__, __LINE__, strerror(errno));
    }

  } while ((res = res->ai_next) != NULL);

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


