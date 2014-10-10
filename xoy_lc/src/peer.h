#ifndef _PEER_H_
#define _PEER_H_

#include <stdint.h>

typedef struct b_peer{

  char ip[16];
  unsigned int port;

  int8_t am_choking;
  int8_t am_interested;
  int8_t peer_choking;
  int8_t peer_interested;

  uint64_t uploaded;
  uint64_t downloaded;

  struct b_peer* next;

} b_peer;

b_peer* b_peer_init();
b_peer* b_peer_init_by_ipport(const char* src);
int16_t b_peer_contain(b_peer* head, b_peer* cur);
void b_peer_free(b_peer* p);

#endif /* end of include guard: _PEER_H_ */
