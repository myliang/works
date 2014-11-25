#ifndef _PEER_H_
#define _PEER_H_

#include <stdint.h>
#include <time.h>
#include "bitmap.h"

#define PEER_STATE_INIT -1
#define PEER_STATE_SEND_HANDSHAKED 0
#define PEER_STATE_RECV_HANDSHAKED 1
#define PEER_STATE_SEND_BITFIELD 2
#define PEER_STATE_RECV_BITFIELD 3
#define PEER_STATE_DATA 4
#define PEER_STATE_CLOSE 5

typedef struct b_peer_request {
  uint32_t index;
  uint32_t begin;
  uint32_t length;

  struct b_peer_request *next;
} b_peer_request;

typedef struct b_peer{

  char ip[16];
  unsigned int port;
  char id[20];

  int8_t am_choking;
  int8_t am_interested;
  int8_t peer_choking;
  int8_t peer_interested;

  uint64_t uploaded;
  uint64_t downloaded;

  int sockfd;
  int state;

  time_t last_time;
  time_t last_downtime;

  bitmap *bitfield;

  b_peer_request *req;

  struct b_peer* next;

} b_peer;

b_peer* b_peer_init();
b_peer* b_peer_init_by_ipport(const char* src);
b_peer* b_peer_init_by_ip_port(const char* ip, unsigned int port);
b_peer* b_peer_has (const char *ip, unsigned int port, b_peer* head);
b_peer* b_peer_contain(b_peer* head, b_peer* cur);
void b_peer_add(b_peer* head, b_peer* cur);
void b_peer_free(b_peer* p);

b_peer_request *b_peer_request_add(b_peer_request *head, uint32_t index, uint32_t begin, uint32_t length);

#endif /* end of include guard: _PEER_H_ */
