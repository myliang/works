#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <stdint.h>

#include <unistd.h>
#include <poll.h>

#include "io_http.h"
#include "io.h"

// 2 ^ 14
#define MAX_IO_BUFFER 1024

static int parse_url(const char* url, io_http_req* req);
static int start_with(char *str, char *key);

char* http_uri_hex(const unsigned char* src, int size) {
  char* begin = malloc(size * 3 + 1);
  char* dst = begin;
  int i = 0;
  for (i = 0; i < 20; i++) {
    sprintf(begin, "%%%.2x", src[i]);
    begin += 3;
  }
  begin[0] = '\0';
  return dst;
}

io_http_res* http_get(const char* url, int timeout) {
  io_http_req req;
  io_http_res* res = malloc(sizeof(io_http_res));
  parse_url(url, &req);
  printf("http.host = %s, http.port = %d, http.path = %s, http.query = %s\n",
      req.host, req.port, req.path, req.query_string);

  int sockfd = io_tcp_connect(req.host, req.port);
  if (sockfd < 0) return NULL;

  // request buff
  char buf[MAX_IO_BUFFER];
  // poll
  struct pollfd pfds[1];
  int maxpfds = 1, nready = 0, n;

  pfds[0].fd = sockfd;
  pfds[0].events = POLLWRNORM;

  for (;;) {
    bzero(buf, sizeof(buf));
    nready = poll(pfds, maxpfds, timeout);
    if (nready < 0) {
      fprintf(stderr, "%s:%d http poll error\n", __FILE__, __LINE__);
      close(sockfd);
      return NULL;
    } else if (nready == 0) {
      fprintf(stderr, "%s:%d http poll timeout\n", __FILE__, __LINE__);
      close(sockfd);
      return NULL;
    }

    if (pfds[0].revents & POLLWRNORM) {
      // write
      // printf("sizeof char[] = %ld\n", sizeof(buf));
      if (req.query_string[0] == '\0')
        snprintf(buf, sizeof(buf), "GET %s HTTP/1.1\r\n", req.path);
      else
        snprintf(buf, sizeof(buf), "GET %s?%s HTTP/1.1\r\n", req.path, req.query_string);

      snprintf(buf + strlen(buf), sizeof(buf), "host: %s\r\n\r\n", req.host);
      printf("request:\n%s\n", buf);
      if (io_writen(sockfd, buf, strlen(buf)) < 0) {
        fprintf(stderr, "%s:%d http send request error: %s\n", __FILE__, __LINE__, buf);
        close(sockfd);
        return NULL;
      }
      pfds[0].events = POLLRDNORM;
    }

    if (pfds[0].revents & POLLRDNORM) {
      // read
      if (io_readline(sockfd, buf, sizeof(buf)) < 0) {
        fprintf(stdout, "%s:%d http recv message error\n", __FILE__, __LINE__);
        close(sockfd);
        return NULL;
      }
      printf("response status line:\n%s\n", buf);
      sscanf(buf, "%s %d %s", res->version, &res->status_code, res->status_message);

      // io_readn(sockfd, buf, 1024);
      // printf("%s\n", buf);
      if (res->status_code == 200) {
        res->content_length = 0;
        while (buf[0] != '\r' && buf[1] != '\n') {
          io_readline(sockfd, buf, sizeof(buf));
          fprintf(stdout, "%s\n", buf);

          // Content-Length
          if (14 == start_with(buf, "Content-Length")) {
            res->content_length = atoi(buf + 15);
            res->content = malloc(res->content_length);
          }
        }
        if (res->content_length <= 0) {
          res->content_length = 8192;
          res->content = malloc(res->content_length);
        }
        n = io_readn(sockfd, res->content, res->content_length);
        res->content[n] = '\0';
        close(sockfd);
        return res;
      }
      break;
    }
  }
  close(sockfd);
  return NULL;
}

void io_http_res_free(io_http_res* res) {
  if (res != NULL) {
    if (res->content != NULL)
      free(res->content);
    free(res);
  }
}

// static methods
static int parse_url(const char* url, io_http_req* req) {
  const char *ptr, *begin;
  int index = 0;
  req->port = 80;
  req->path[0] = '/';
  req->path[1] = '\0';
  req->query_string[0] = '\0';

  ptr = url + 5;
  // for (ptr = url + 5; *ptr != '\0'; ptr++) {
  if (*ptr == '/' && *(ptr + 1) == '/') { // = host
    ptr++;
    begin = ++ptr;
    while (*ptr != '\0' && *ptr != '/' && *ptr != ':'){
      index++;
      ptr++;
    }
    memcpy(req->host, begin, index);
    req->host[index] = '\0';
  }

  if (*ptr == ':') { // = port
    index = 0;
    ptr++;
    for (; isdigit(*ptr); ptr++) {
      index = index * 10 + (*ptr - '0');
    }
    req->port = index;
  }

  if (*ptr == '/') { // = path
    begin = ptr;
    index = 0;
    while (*ptr != '?' && *ptr != '\0') {
      index++;
      ptr++;
    }
    memcpy(req->path, begin, index);
    req->path[index] = '\0';
  }

  if (*ptr == '?') {
    index = 0;
    begin = ++ptr;
    for (; *ptr != '\0'; ptr++) {
      index++;
    }
    memcpy(req->query_string, begin, index);
    req->query_string[index] = '\0';
  }
  // }
  return 1;
}

static int start_with(char *str, char *key) {
  int i = 0;
  while (*key != '\0' && *str != '\0') {
    if (*key++ != *str++)
      return i;
    i++;
  }
  return i;
}
