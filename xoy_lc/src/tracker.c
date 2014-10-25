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
#include <arpa/inet.h>
#include <netinet/in.h>

#include "config.h"
#include "bencode.h"
#include "torrent.h"
#include "tracker.h"
#include "io.h"
#include "io_http.h"

#define TRACKER_NUMWANT 100
#define UDP_CONNECTION_ID 0x41727101980

enum EVENT {
  NONE,
  COMPLETED,
  STARTED,
  STOPPED
};

static char *tracker_events[] = {
  "none",
  "completed",
  "started",
  "stopped"
};

#define EVENT_COMPLETED tracker_events[COMPLETED]
#define EVENT_STARTED tracker_events[STARTED]
#define EVENT_STOPPED tracker_events[STOPPED]

#define UDP_SEND(sockfd, buf) \
  if (send(sockfd, buf, sizeof(buf), 0) < 0) { \
    fprintf(stderr, "%s:%d udp send error: %s\n", __FILE__, __LINE__, strerror(errno)); \
    close(sockfd); \
    return 0; \
  }
#define UDP_RECV(sockfd, buf) \
  if (recv(sockfd, buf, sizeof(buf), 0) < 0) { \
    fprintf(stderr, "%s%d udp recv error: %s\n", __FILE__, __LINE__, strerror(errno)); \
    close(sockfd); \
    return 0; \
  }

static int request_trackers_with_http(const char* url, b_torrent* tptr, int timeout);
static int request_trackers_with_udp(const char* url, b_torrent* tptr, int timeout);
static void parse_udp_url(const char* url, char* host, short* port);
static int udp_connect_with_url(const char* url);

void request_trackers(b_torrent* tptr, int timeout) {
  b_torrent_tracker* tracker = tptr->tracker;
  const char* url;
  while (tracker != NULL) {
    url = tracker->url;
    printf("tracker url: %s\n", url);
    if (url[0] == 'h') { // http tracker
      if (request_trackers_with_http(url, tptr, timeout) == 1)
        return ;
    } else if(url[0] == 'u') { // udp tracker
      if (request_trackers_with_udp(url, tptr, timeout) == 1)
        return ;
    }
    tracker = tracker->next;
  }
}

// static methods
static int request_trackers_with_http(const char* url, b_torrent* tptr, int timeout) {
  char full_url[1024];
  int i;
  snprintf(full_url, sizeof(full_url) - 1,
      "%s?info_hash=%s&peer_id=%s&port=%d&uploaded=%llu&downloaded=%llu&left=%llu&event=%s&numwant=%d&compact=1",
      url, http_uri_hex(tptr->info_hash, 20), http_uri_hex(tptr->peer_id, 20),
      BT_LISTEN_PORT, (unsigned long long)tptr->uploaded, (unsigned long long)tptr->downloaded, (unsigned long long)tptr->left, EVENT_STARTED, TRACKER_NUMWANT);
  full_url[sizeof(full_url) - 1] = '\0';
  // printf("url=%s\n", full_url);

  io_http_res* res = http_get(full_url, timeout);
  if (res == NULL) {
    printf("http response is null\n");
    return 0;
  }

  // format response content
  b_buffer* buf = b_buffer_init_with_string(res->content, res->content_length);
  b_encode* be = b_encode_init(buf);


  b_dict* bdict = be->data.dpv;
  while (bdict != NULL) {
    if (strncmp("peers", bdict->key, max(5, strlen(bdict->key))) == 0) {
      char *index = bdict->value->data.cpv;
      b_peer bprhead;
      b_peer *bpr = &bprhead;
      for (i = 0; i < bdict->value->len; i += 6) {
        // printf("%u.%u.%u.%u\n", (unsigned char)index[i + 0], (unsigned char)index[i + 1], (unsigned char)index[i + 2], (unsigned char)index[i + 3]);
        b_peer *bp1 = b_peer_init_by_ipport(index + i);
        if (b_peer_contain(tptr->peer, bp1) == 0) {
          bpr = bpr->next = bp1;
          tptr->peer_len++;
        }
      }

      if (tptr->peer == NULL) tptr->peer = bprhead.next;
      else tptr->peer->next = bprhead.next;
      return 1;
    }
    bdict = bdict->next;
  }

  io_http_res_free(res);
  b_encode_free(be, buf);

  return 0;
}

// req: connection_id(8 bytes) 0 (4 bytes) transaction_id(4 bytes)
// res: 0(4 bytes) transaction_id(4 bytes) connection_id(8 bytes)
// req: info_hash(20 bytes) peer_id(20 bytes) download(8 bytes) left(8 bytes) upload(8 bytes) event(4 bytes)
//      ip(4 bytes) 0(8 bytes) port(2 bytes)
// res: 1(4 bytes) transaction_id(4 bytes) interval(4 bytes) downalod(4 bytes) peers(4 bytes) ip(4 bytes) port (2)
static int request_trackers_with_udp(const char* url, b_torrent* tptr, int timeout) {
  // request message
  unsigned char buf[16], buf1[98];
  unsigned char recvbuf[2048];
  unsigned char *begin = buf;
  srand_curr_time;
  uint32_t transaction_id = rand();

  int i;

  // socket
  int sockfd = udp_connect_with_url(url);
  if (sockfd < 0) return 0;

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
      return 0;
    } else if (n < 0) {
      fprintf(stderr, "%s:%d udp select error: %s\n", __FILE__, __LINE__, strerror(errno));
      close(sockfd);
      return 0;
    }

    if (FD_ISSET(sockfd, &fdset)) {
      switch (rwflag) {
        case 0:
          int2bytes8(begin, UDP_CONNECTION_ID);
          int2byte(begin + 8, 0x00);
          int2byte(begin + 9, 0x00);
          int2byte(begin + 10, 0x00);
          int2byte(begin + 11, 0x00);
          int2bytes4(begin + 12, transaction_id);

          for (i = 0; i < 16; i++) {
            printf("%.2x ", buf[i]);
          }
          printf("\n");

          UDP_SEND(sockfd, buf);
          printf("write\n");
          break ;
        case 1:
          UDP_RECV(sockfd, recvbuf);
          begin = recvbuf;
          if (bytes42int(begin) == 0 && bytes42int(begin + 4) == transaction_id) {
            printf("udp success: %s\n", recvbuf);
            break;
          }
          printf("udp response vidate error \n");
          return 0;
        case 2:
          begin = buf1;
          int2bytes8(begin, UDP_CONNECTION_ID);
          int2bytes4(begin + 8, 0x01);
          int2bytes4(begin + 12, transaction_id);
          memcpy(begin + 16, tptr->info_hash, 20);
          memcpy(begin + 36, tptr->peer_id, 20);
          int2bytes8(begin + 56, tptr->downloaded);
          int2bytes8(begin + 64, tptr->left);
          int2bytes8(begin + 72, tptr->uploaded);
          int2bytes4(begin + 80, STARTED);
          int2bytes4(begin + 84, 0x00);
          int2bytes8(begin + 88, (long long)0x00);
          int2bytes2(begin + 96, LISTEN_PORT);
          UDP_SEND(sockfd, buf1);
          break ;
        case 3:
          UDP_RECV(sockfd, recvbuf);
          begin = recvbuf;
          if (bytes42int(begin) == 1 && bytes42int(begin) == transaction_id) {
            printf("udp success: %s\n", recvbuf);
            return 1;
          }
          printf("udp reponse validate error\n");
          return 0;

      }

    }

    rwflag++;

  }

  close(sockfd);
  return 0;
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

