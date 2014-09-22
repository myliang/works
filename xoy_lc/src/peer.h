#ifndef _PEER_H_
#define _PEER_H_

#include <stdint.h>

typedef struct b_peer{

  char ip[20];
  unsigned int port;

  // int16_t am_choking = 1;
  // int16_t am_interested = 0;
  // int16_t peer_choking = 1;
  // int16_t peer_interested = 0;

  struct b_peer* next;

} b_peer;

b_peer* b_peer_init();
int16_t b_peer_contain(b_peer* head, b_peer* cur);
void b_peer_add_ip_port(b_peer* cur, const char* src);
void b_peer_free(b_peer* p);

#endif /* end of include guard: _PEER_H_ */