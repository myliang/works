#ifndef _IO_HTTP_H_
#define _IO_HTTP_H_

typedef struct {
  char host[120];
  short port;
  char path[100];
  char query_string[2048];
} io_http_req;

typedef struct {
  char version[20]; // http version
  int status_code;
  char status_message[20]; // short message
  char server[100]; // response server name
  int content_length;
  char content_type[100]; // content type
  char *content; // response content
} io_http_res;

char* http_uri_hex(const unsigned char* uri, int size);
char* http_get(const char* url, int timeout);

#endif /* end of include guard: _IO_HTTP_H_ */
