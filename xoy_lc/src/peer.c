#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>

#include "peer.h"


b_peer* b_peer_init () {
  b_peer* p = malloc(sizeof(b_peer));
  p->next = NULL;
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

void b_peer_add_ip_port (b_peer* cur, const char* src) {
  int64_t ip = (src[0] << 24) + (src[1] << 16) + (src[2] << 8) + src[3];
  struct in_addr s;
  s.s_addr = ip;
  inet_ntop(AF_INET, (void*)&s, cur->ip, 16);
  cur->port = (src[4] << 8 & 0xffff) + src[5];
  printf("peer->ip = %s, peer->port = %d\n", cur->ip, cur->port);
}

void b_peer_free(b_peer* p) {
  free(p);
}

