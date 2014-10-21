#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>

#include "peer.h"
#include "config.h"
#include "io.h"


b_peer* b_peer_init () {
  b_peer* p = malloc(sizeof(b_peer));

  p->next = NULL;
  p->state = PEER_STATE_INIT;

  p->am_choking = 1;
  p->am_interested = 0;
  p->peer_choking = 1;
  p->peer_interested = 0;

  return p;
}

int16_t b_peer_contain (b_peer* head, b_peer* cur) {
  while (NULL != head) {
    if (strcmp(head->ip, cur->ip) == 0 && head->port == cur->port) {
      return 1;
    }
    head = head->next;
  }
  return 0;
}

b_peer* b_peer_init_by_ipport (const char* src) {
  b_peer *bp = b_peer_init();
  struct in_addr addr;

  addr.s_addr = bytes42int(src);
  bp->port = bytes22int(src + 4);
  memcpy(bp->ip, inet_ntoa(addr), 16);
  return bp;
}

void b_peer_free(b_peer* p) {
  free(p);
}


