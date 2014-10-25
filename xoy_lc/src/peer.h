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

  struct b_peer* next;

} b_peer;

b_peer* b_peer_init();
b_peer* b_peer_init_by_ipport(const char* src);
int16_t b_peer_contain(b_peer* head, b_peer* cur);
void b_peer_free(b_peer* p);

#endif /* end of include guard: _PEER_H_ */
