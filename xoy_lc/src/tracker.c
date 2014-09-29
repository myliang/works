#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <unistd.h>

#include <sys/socket.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <errno.h>

#include "config.h"
#include "bencode.h"
#include "torrent.h"
#include "tracker.h"
#include "io.h"
#include "io_http.h"

#define TRACKER_NUMWANT 200

static char *tracker_events[] = {
  "started",
  "stopped",
  "completed"
};

static void request_trackers_with_http(const char* url, b_torrent* tptr, b_peer* pptr, int timeout);
static void request_trackers_with_udp(const char* url, b_torrent* tptr, b_peer* pptr, int timeout);
static void parse_udp_url(const char* url, char* host, short* port);
static int udp_connect_with_url(const char* url);

void request_trackers(b_torrent* tptr, b_peer* pptr, int timeout) {
  b_torrent_tracker* tracker = tptr->tracker;
  const char* url;
  while (tracker != NULL) {
    url = tracker->url;
    printf("tracker url: %s\n", url);
    if (url[0] == 'h') { // http tracker
      request_trackers_with_http(url, tptr, pptr, timeout);
    } else if(url[0] == 'u') { // udp tracker
      request_trackers_with_udp(url, tptr, pptr, timeout);
    }
    tracker = tracker->next;
  }
}

// static methods
static void request_trackers_with_http(const char* url, b_torrent* tptr, b_peer* pptr, int timeout) {
  char full_url[1024];
  snprintf(full_url, sizeof(full_url) - 1,
      "%s?info_hash=%s&peer_id=%s&port=%d&uploaded=%d&downloaded=%d&left=%d&event=%s&numwant=%d&compact=1",
      url, http_uri_hex(tptr->info_hash, 20), http_uri_hex(tptr->peer_id, 20),
      LISTEN_PORT, 0, 0, 10240000, tracker_events[0], TRACKER_NUMWANT);
  full_url[sizeof(full_url) - 1] = '\0';
  // printf("url=%s\n", full_url);

  io_http_res* res = http_get(full_url, timeout);
  if (res == NULL) {
    printf("http response is null\n");
    return ;
  }

  printf("http response : %s\n", res->content);

  // format response content
  b_buffer* buf = b_buffer_init_with_string(res->content, res->content_length);
  b_encode* be = b_encode_init(buf);

  b_encode_print(be);

  b_encode_free(be, buf);

}

// req: connection_id(8 bytes) 0 (4 bytes) transaction_id(4 bytes)
// res: 0(4 bytes) transaction_id(4 bytes) connection_id(8 bytes)
// req: info_hash(20 bytes) peer_id(20 bytes) download(8 bytes) left(8 bytes) upload(8 bytes) event(4 bytes)
//      ip(4 bytes) 0(8 bytes) port(4 bytes)
// res: 1(4 bytes) transaction_id(4 bytes) interval(4 bytes) downalod(4 bytes) peers(4 bytes) ip(4 bytes) port (2)
static void request_trackers_with_udp(const char* url, b_torrent* tptr, b_peer* pptr, int timeout) {
  // request message
  char buf[16];
  char recvbuf[2048];
  char* index = buf, *end;
  int2bytes8(index, 0x41727101980);
  end = index + 4;
  int2byte(index, end, 0x00);
  srand_curr_time;
  uint32_t transaction_id = rand();
  int2bytes4(index, transaction_id);

  // socket
  int sockfd = udp_connect_with_url(url);
  if (sockfd < 0) return ;

  int maxfd, n, rwflag = 0; // 0 write, 1 read, 2 write, 3 read
  struct timeval tv;
  fd_set fdset;

  tv.tv_sec = timeout;
  tv.tv_usec = 0;

  for (;;) {
    FD_ZERO(&fdset);
    FD_SET(sockfd, &fdset);
    maxfd = sockfd + 1;
    // printf("before.maxfd = %d, nready = %d, rwflag = %d\n", maxfd, n, rwflag);
    if (rwflag%2 == 0)
      n = select(maxfd, NULL, &fdset, NULL, &tv);
    else
      n = select(maxfd, &fdset, NULL, NULL, &tv);

    printf("maxfd = %d, nready = %d, rwflag = %d\n", maxfd, n, rwflag);

    if (n == 0) {
      fprintf(stderr, "%s:%d udp select timeout\n", __FILE__, __LINE__);
      close(sockfd);
      return ;
    } else if (n < 0) {
      fprintf(stderr, "%s:%d udp select error: %s\n", __FILE__, __LINE__, strerror(errno));
      close(sockfd);
      return ;
    }

    if (FD_ISSET(sockfd, &fdset)) {
      switch (rwflag) {
        case 0:
          if (send(sockfd, buf, sizeof(buf), 0) < 0) {
            fprintf(stderr, "%s:%d udp send error: %s\n", __FILE__, __LINE__, strerror(errno));
            close(sockfd);
            return ;
          }
          printf("write\n");
          break ;
        case 1:
          if (recv(sockfd, recvbuf, sizeof(recvbuf), 0) < 0) {
            fprintf(stderr, "%s%d udp recv error: %s\n", __FILE__, __LINE__, strerror(errno));
            close(sockfd);
            return ;
          }
          printf("udp response: %s\n", recvbuf);
          break ;
        case 2:
          break ;
        case 3:
          return ;
          break ;

      }

    }

    rwflag++;

  }

  close(sockfd);
}

static int udp_connect_with_url(const char* url) {
  char host[16];
  short port = 80;
  parse_udp_url(url, host, &port);
  printf("host = %s, port = %d\n", host, port);
  int sockfd = io_udp_connect(host, port);
  return sockfd;
}
static void parse_udp_url(const char* url, char* host, short* port) {
  const char *ptr, *begin;
  int index = 0;

  ptr = url + 4;
  // for (ptr = url + 4; *ptr != '\0';) {
  if (*ptr == '/' && *(ptr + 1) == '/') { // = host
    ptr++;
    begin = ++ptr;
    while (*ptr != '\0' && *ptr != '/' && *ptr != ':'){
      index++;
      ptr++;
    }
    memcpy(host, begin, index);
    host[index] = '\0';
  }

  if (*ptr == ':') { // = port
    index = 0;
    ptr++;
    for (; isdigit(*ptr); ptr++) {
      index = index * 10 + (*ptr - '0');
    }
    *port = index;
  }
  // }
}

