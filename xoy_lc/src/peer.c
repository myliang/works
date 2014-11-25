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

  p->last_time = 0;
  p->last_downtime = 0;

  p->uploaded = 0;
  p->downloaded = 0;

  return p;
}

b_peer* b_peer_has (const char *ip, unsigned int port, b_peer* head) {
  while (NULL != head) {
    if (strcmp(head->ip, ip) == 0 && head->port == port) {
      return head;
    }
    head = head->next;
  }
  return NULL;
}

b_peer* b_peer_contain (b_peer* head, b_peer* cur) {
  return b_peer_has(cur->ip, cur->port, head);
}

b_peer* b_peer_init_by_ipport (const char* src) {
  b_peer *bp = b_peer_init();
  struct in_addr addr;

  addr.s_addr = bytes42int(src);
  bp->port = bytes22int(src + 4);
  memcpy(bp->ip, inet_ntoa(addr), 16);
  return bp;
}

b_peer* b_peer_init_by_ip_port(const char* ip, unsigned int port) {
  b_peer *bp = b_peer_init();
  bp->port = port;
  memcpy(bp->ip, ip, 16);
  return bp;
}

void b_peer_add(b_peer* head, b_peer* cur) {
  b_peer* tail = head;
  while (NULL != head) {
    tail = head;
    head = head->next;
  }
  tail->next = cur;
}

void b_peer_free(b_peer* p) {
  free(p);
}

b_peer_request *b_peer_request_add(b_peer_request *head, uint32_t index, uint32_t begin, uint32_t length) {
  b_peer_request *tmp = head;
  b_peer_request *tail = head;
  b_peer_request *curr = malloc(sizeof(struct b_peer_request));

  curr->index = index;
  curr->begin = begin;
  curr->length = length;

  while (tmp != NULL) {
    // if contain request
    if (tmp->index == index && tmp->begin == begin && tmp->length == length)
      return head;

    tail = tmp;
    tmp = tmp->next;
  }

  if (tail == NULL) head = curr;
  else tail->next = curr;

  return head;
}
