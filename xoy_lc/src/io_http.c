#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

#include "io_http.h"
#include "io.h"

typedef struct {
  char host[120];
  short port;
  char path[100];
  char query_string[2048];
} io_http;

static int parse_url(const char* url, io_http* h);

int http_get(const char* url, int timeout, char* res, int resn) {
  io_http hp;
  parse_url(url, &hp);
  return -1;
}

// static methods
static int parse_url(const char* url, io_http* h) {
  const char *ptr, *begin;
  int index = 0;
  io_http http;
  http.port = 80;

  for (ptr = url; *ptr != '\0'; ptr++) {
    if (*ptr == ':' && *++ptr == ':') { // = host
      begin = ++ptr;
      while (*ptr != '\0' || *ptr != '/'){
        index++;
        ptr++;
      }
      memcpy(http.host, begin, index);
      http.host[index] = '\0';
    } else if (*ptr == ':') { // = port
      index = 0;
      ptr++;
      for (; isdigit(*ptr); ptr++) {
        index = index * 10 + (*ptr - '0');
      }
      http.port = index;
    } else if (*ptr == '/') { // = path
      begin = ptr;
      index = 0;
      while (*ptr != '?' || *ptr != '\0') {
        index++;
        ptr++;
      }
      memcpy(http.path, begin, index);
      http.path[index] = '\0';
    } else if (*ptr == '?') {
      index = 0;
      begin = ++ptr;
      for (; *ptr != '\0'; ptr++) {
        index++;
      }
      memcpy(http.query_string, begin, index);
      http.query_string[index] = '\0';
    }
  }
  return 1;
}
