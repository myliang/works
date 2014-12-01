#include "peer_wire.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <poll.h>
#include <errno.h>
#include <arpa/inet.h>

#include "config.h"
#include "io.h"
#include "torrent.h"

#define UPDATE_LAST_TIME(ptr) (ptr)->last_time = time(NULL)
#define UPDATE_LAST_RECVTIME(ptr) (ptr)->last_recvtime = time(NULL)

#define MESSAGE_CHOKE 0
#define MESSAGE_UNCHOKE 1
#define MESSAGE_INTERESTED 2
#define MESSAGE_NOTINTERESTED 3
#define MESSAGE_HAVE 4
#define MESSAGE_BITFIELD 5
#define MESSAGE_REQUEST 6
#define MESSAGE_PIECE 7
#define MESSAGE_CANCEL 8
// #define MESSAGE_KEEPALIVE 9

#define MAX_POLLFD 1024
//
typedef int (*Function)(const char *buf, b_peer *bp, b_torrent *bt);

static int mrecv_handshake(const char *buf, b_peer *bp, b_torrent *bt);
static int mrecv_keepalive(const char *buf, b_peer *bp, b_torrent *bt);
static int mrecv_choke(const char *buf, b_peer *bp, b_torrent *bt);
static int mrecv_unchoke(const char *buf, b_peer *bp, b_torrent *bt);
static int mrecv_interested(const char *buf, b_peer *bp, b_torrent *bt);
static int mrecv_notinterested(const char *buf, b_peer *bp, b_torrent *bt);
static int mrecv_have(const char *buf, b_peer *bp, b_torrent *bt);
static int mrecv_bitfield(const char *buf, b_peer *bp, b_torrent *bt);
static int mrecv_request(const char *buf, b_peer *bp, b_torrent *bt);
static int mrecv_piece(const char *buf, b_peer *bp, b_torrent *bt);
static int mrecv_cancel(const char *buf, b_peer *bp, b_torrent *bt);

static Function message_functions[] = {
  mrecv_choke, mrecv_unchoke, mrecv_interested, mrecv_notinterested, mrecv_have,
  mrecv_bitfield, mrecv_request, mrecv_piece, mrecv_cancel
};

// handshake : <pstrlen><pstr><reserved><info_hash><peer_id>
static size_t message_handshake(char *dst, const unsigned char *info_hash, const char *peer_id);
// keep-alive: <len=0000>
static size_t message_keepalive(char *dst);
// choke: <len=0001><id=0>
static size_t message_choke(char *dst);
// unchoke: <len=0001><id=1>
static size_t message_unchoke(char *dst);
// interested: <len=0001><id=2>
static size_t message_interested(char *dst);
// not interested: <len=0001><id=3>
static size_t message_notinterested(char *dst);
// have: <len=0005><id=4><piece index>
static size_t message_have(char *dst, uint32_t index);
// bitfield: <len=0001+X><id=5><bitfield>
// X : bitfield len
static size_t message_bitfield(char *dst, bitmap *bm);
// request: <len=0013><id=6><index><begin><length>
// index: piece index
// begin: offset of piece
// length = 16KB
static size_t message_request(char *dst, uint32_t index, uint32_t begin, uint32_t length);
// piece: <len=0009+X><id=7><index><begin><block>
// X : block len , default 16KB
static size_t message_piece(char *dst, uint32_t index, uint32_t begin, char* block);
// cancel: <len=0013><id=8><index><begin><length>
static size_t message_cancel(char *dst, uint32_t index, uint32_t begin, uint32_t length);
// port: <len=0003><id=9><listen-port>
// size_t message_port(char *dst, uint16_t port);

// global variable
static b_peer* cache_peers[MAX_CONNECTIONS];

void b_peer_wire_downup_message(b_torrent *bt, int timeout) {
  time_t now;
  struct pollfd fds[MAX_POLLFD];
  struct sockaddr_in clients[MAX_POLLFD]; // cache connection client ip and port
  int pollfdlen = MAX_POLLFD, nready = 0, i, connfd, nread = 0;
  socklen_t clilen;
  struct sockaddr_in cliaddr;
  char readbuff[BT_PEER_WIRE_BUFFER_LEN];

  int listenfd = io_tcp_listen("localhost", BT_LISTEN_PORT);
  fds[0].fd = listenfd;
  fds[0].events = POLLIN;

  for (i = 1; i < MAX_POLLFD; i++) {
    fds[i].fd = -1;
  }

  for (i = 0; i < MAX_CONNECTIONS; i++) {
    cache_peers[i] = NULL;
  }

  pollfdlen = 1;

  for (;;) {
    now = time(NULL);

    // pick peers with unchoked every ten second
    //
    // redo pick and option peers with unchoked every 30 second
    //
    // request tracker every 5 mintue
    //
    // create peer connction
    nready = poll(fds, pollfdlen, timeout);
    if (nready == 0) continue;
    else if (nready < 0) {
      fprintf(stderr, "%s:%d poll eror %s\n", __FILE__, __LINE__, strerror(errno));
      break;
    }

    if (fds[0].revents & POLLIN) {
      clilen = sizeof(cliaddr);
      connfd = accept(listenfd, (struct sockaddr *)&cliaddr, &clilen);
      if (connfd < 0) continue;
      printf("%s:%d accept client.ip=%s, client.port=%d\n", __FILE__, __LINE__, inet_ntoa(cliaddr.sin_addr), cliaddr.sin_port);

      for (i = 1; i < MAX_POLLFD; i++) {
        if (fds[i].fd < 0) {
          fds[i].fd = connfd;
          fds[i].events = POLLIN;
          memcpy(&clients[i], &cliaddr, clilen);
          break ;
        }
      }

      if (i == MAX_POLLFD) {
        printf("too many connection\n");
        continue ;
      }

      if (i > pollfdlen)
        pollfdlen = i;

      if (--nready <= 0)
        continue ;
    }

    for (i = 1; i < pollfdlen; i++) {
      connfd = fds[i].fd;
      if (connfd < 0) continue ;

      if (fds[i].revents & (POLLIN | POLLERR)) {
        nread = io_readn(connfd, readbuff, sizeof(readbuff));
        if ((nread < 0 && errno == ECONNRESET) || (nread == 0)) {
          close(connfd);
          fds[i].fd = -1;
          bzero(&clients[i], sizeof(struct sockaddr_in));
        }

        // recv message
        b_peer_wire_recv_message(readbuff, bt, connfd, &clients[i]);

      }

      if (--nready <= 0)
        continue ;
    }

    if (fds[i].revents & (POLLOUT | POLLWRNORM)) {
      b_peer_wire_send_message(cache_peers[connfd], bt);
    }

  }
}

void b_peer_wire_message(b_torrent *bt) {
  b_peer *bp = bt->peer;
  if (bp == NULL) return ;

  size_t len;
  char buf[96];

  while (bp != NULL) {
    // bp->ip, bp-port
    if (bp->sockfd < 0) {
      bp->sockfd = io_tcp_connect(bp->ip, bp->port);
    }
    // hande shake
    if (bp->sockfd > 0) {
      len = message_handshake(buf, bt->info_hash, bt->peer_id);
      io_writen(bp->sockfd, buf, len);
    }

    bp = bp->next;
  }
}

int b_peer_wire_send_message(b_peer* bp, b_torrent *bt) {
  if (bp == NULL) return -1;
  char buf[100];
  size_t len;
  if (bp->state == PEER_STATE_INIT) {
    len = message_handshake(buf, bt->info_hash, bt->peer_id);
    bp->state = PEER_STATE_SEND_HANDSHAKED;
  }

  if (bp->state == PEER_STATE_RECV_HANDSHAKED) {
    len = message_bitfield(buf, bt->bitfield);
    bp->state = PEER_STATE_SEND_BITFIELD;
  }

  // upload data
  if (bp->am_choking == 0 && bp->peer_interested == 1) {
    b_peer_request *req = bp->req;
    char dst[BT_PEER_WIRE_BUFFER_LEN];
    while (req != NULL) {
      // read data to remote client from disk
      char block[req->length];
      b_torrent_file_read(req, block, bt);
      len = message_piece(dst, req->index, req->begin, block);
      io_writen(bp->sockfd, dst, len);
      req = req->next;
    }
  }

  time_t now = time(NULL);
  long interval = now = bp->last_time;
  if (interval > 180) {
    bp->state = PEER_STATE_CLOSE;
    close(bp->sockfd);
  } else if(interval > 45) {
    len = message_keepalive(buf);
    io_writen(bp->sockfd, buf, len);
  }

  return 0;
}

int b_peer_wire_recv_message(const char *buf, b_torrent *bt, int sockfd, struct sockaddr_in *client) {
  b_peer *bp = NULL;
  b_torrent *curr = bt;

  // hand shake message
  if (buf[0] == 19 && strncmp(buf + 1, bittorrent_protocol, 19) == 0) {
    while (curr != NULL) {
      if (strncmp(buf + 20, curr->info_hash, 20) == 0) {
        char *ip = inet_ntoa(client->sin_addr);
        bp = b_peer_has(ip, client->sin_port, curr->peer);
        if (bp == NULL) {
          bp = b_peer_init_by_ip_port(ip, client->sin_port);
          b_peer_add(curr->peer, bp);
        }
        bp->sockfd = sockfd;
        cache_peers[sockfd] = bp;
        return mrecv_handshake(buf, bp, bt);
      }
      curr = curr->next;
    }

    printf("%s:%d torrent don't exists", __FILE__, __LINE__);
    close(sockfd);
  } else if (bytes42int(buf) == 0 && cache_peers[sockfd] != NULL) {
    return mrecv_keepalive(buf, cache_peers[sockfd], bt);
  } else if (message_functions[buf[4]] != NULL && cache_peers[sockfd] != NULL) {
    return message_functions[buf[4]](buf, cache_peers[sockfd], bt);
  } else {
    fprintf(stderr, "recv message error\n");
    return -1;
  }

  return 0;
}

// message recv
static int mrecv_handshake(const char *buf, b_peer *bp, b_torrent *bt) {
  char sendbuf[100];
  size_t len;
  memcpy(bp->id, buf + 48, 20);
  if (bp->state == PEER_STATE_INIT) {
    // send handshake
    len = message_handshake(sendbuf, bt->info_hash, bt->peer_id);
    if (io_writen(bp->sockfd, sendbuf, len) < 0) return 68;
  }
  bp->state = PEER_STATE_RECV_HANDSHAKED;
  UPDATE_LAST_TIME(bp);
  return 68;
}
static int mrecv_keepalive(const char *buf, b_peer *bp, b_torrent *bt) {
  UPDATE_LAST_TIME(bp);
  return 4;
}
static int mrecv_choke(const char *buf, b_peer *bp, b_torrent *bt) {
  if (bp->state == PEER_STATE_DATA && bp->peer_choking == 0) {
    bp->peer_choking = 1;
    // bp->last_recvtime = 0;
    bp->downloaded = 0;
  }
  UPDATE_LAST_TIME(bp);
  return 5;
}
static int mrecv_unchoke(const char *buf, b_peer *bp, b_torrent *bt) {
  if (bp->state == PEER_STATE_DATA && bp->peer_choking == 1) {
    bp->peer_choking = 0;
    if (bp->am_interested == 0) {
      int ret[] = {0, 0};
      bitmap_compare(ret, bp->bitfield, bt->bitfield);
      if (ret[0] > 0)
        bp->am_interested = 1;
      else
        printf("Received unchoke but Not interested to IP:%s\n", bp->ip);
    }

    if (bp->am_interested == 1) { /** todo **/  }
    // bp->last_recvtime = 0;
    bp->downloaded = 0;
  }
  UPDATE_LAST_TIME(bp);
  return 5;
}
static int mrecv_interested(const char *buf, b_peer *bp, b_torrent *bt) {
  int ret[] = {0, 0};
  char sendbuf[100];
  int len;

  if (bp->state == PEER_STATE_DATA) {
    bitmap_compare(ret, bp->bitfield, bt->bitfield);
    if (ret[1] > 0) bp->peer_interested = 1;
    if (bp->peer_interested == 0) return 5;
    if (bp->am_choking == 0) {
      // send unchoke
      len = message_unchoke(sendbuf);
      io_writen(bp->sockfd, buf, len);
    }
  }
  UPDATE_LAST_TIME(bp);
  return 5;
}
static int mrecv_notinterested(const char *buf, b_peer *bp, b_torrent *bt) {
  if (bp->state == PEER_STATE_DATA)
    bp->peer_interested = 0;
  // cancel requested
  UPDATE_LAST_TIME(bp);
  return 5;
}
static int mrecv_have(const char *buf, b_peer *bp, b_torrent *bt) {
  int rnum, len;
  int ret[] = {0, 0};
  srand_curr_time;
  char sendbuf[100];

  rnum = rand() % 3;
  if (bp->state == PEER_STATE_DATA) {
    bitmap_set(bp->bitfield, bytes42int(buf + 5));
    if (bp->am_interested == 0) {
      bitmap_compare(ret, bp->bitfield, bt->bitfield);
      if (ret[1] > 0){
        bp->am_interested = 1;
        // send interested message
        len = message_interested(sendbuf);
        io_writen(bp->sockfd, sendbuf, len);
      }
    } else {
      if (rnum == 0){
        len = message_interested(sendbuf);
        io_writen(bp->sockfd, sendbuf, len);
      }
    }
  }
  UPDATE_LAST_TIME(bp);
  return 9;
}
static int mrecv_bitfield(const char *buf, b_peer *bp, b_torrent *bt) {
  int bitlen = bt->bitfield->len;
  int ret[] = {0, 0};
  int len;

  if (bp->state == PEER_STATE_RECV_HANDSHAKED || bp->state == PEER_STATE_SEND_BITFIELD) {
    if (bp->bitfield != NULL) {
      bitmap_free(bp->bitfield);
      bp->bitfield = NULL;
    }

    if (bytes42int(buf) - 1 != bitlen) {
      bp->state = PEER_STATE_CLOSE;
      close(bp->sockfd);
      return 5 + bitlen;
    }

    bp->bitfield = bitmap_init(buf + 5, bitlen);
    char sendbuf[bitlen + 5];

    if (bp->state == PEER_STATE_RECV_HANDSHAKED) {
      len = message_bitfield(sendbuf, bt->bitfield);
      io_writen(bp->sockfd, sendbuf, len);
      // bp->state = PEER_STATE_DATA;
    }

    // if (bp->state == PEER_STATE_SEND_BITFIELD)
    bp->state = PEER_STATE_DATA;

    // interested
    bitmap_compare(ret, bp->bitfield, bt->bitfield);
    if (ret[0] > 0) {
      len = message_interested(sendbuf);
      io_writen(bp->sockfd, sendbuf, len);
      bp->am_interested = 1;
    }
    if (ret[1] > 0) bp->peer_interested = 1;
  }
  UPDATE_LAST_TIME(bp);
  return 5 + len;
}
static int mrecv_request(const char *buf, b_peer *bp, b_torrent *bt) {
  // todo reqeust quene
  if (bp->am_choking == 0 && bp->peer_interested == 1) {
    uint32_t index = bytes42int(buf + 5);
    uint32_t begin = bytes42int(buf + 9);
    uint32_t length = bytes42int(buf + 13);
    bp->req = b_peer_request_add(bp->req, index, begin, length);
  }
  UPDATE_LAST_TIME(bp);
  return 17;
}
static int mrecv_piece(const char *buf, b_peer *bp, b_torrent *bt) {
  uint32_t length = bytes42int(buf) - 0x09;

  if (bp->am_choking == 0 && bp->peer_interested == 1) {
    uint32_t index = bytes42int(buf + 5);
    uint32_t begin = bytes42int(buf + 9);

    bp->res = b_peer_response_add(bp->res, index, begin, length, buf + 13);

    if (bp->last_downtime == 0) bp->last_downtime = time(NULL);
    bp->downloaded += length;

    // write disk from res
    b_torrent_store_all(bp, bt);
  }

  UPDATE_LAST_TIME(bp);
  return length + 13;
}
static int mrecv_cancel(const char *buf, b_peer *bp, b_torrent *bt) {
  UPDATE_LAST_TIME(bp);
  return 17;
}

// static methods
static size_t message_handshake(char *dst, const unsigned char *info_hash, const char *peer_id)  {
  dst[0] = sizeof(bittorrent_protocol) - 1;
  memcpy(dst + 1, bittorrent_protocol, sizeof(bittorrent_protocol) - 1);
  memcpy(dst + 20, "00000000", 8);
  memcpy(dst + 28, info_hash, 20);
  memcpy(dst + 48, peer_id, 20);
  return 68;
}

static size_t message_keepalive(char *dst) {
  int2bytes4(dst, 0x00);
  return 4;
}

static size_t message_choke(char *dst) {
  int2bytes4(dst, 0x01);
  int2byte(dst + 4, 0x00);
  return 5;
}

static size_t message_unchoke(char *dst) {
  int2bytes4(dst, 0x01);
  int2byte(dst + 4, 0x01);
  return 5;
}

static size_t message_interested(char *dst) {
  int2bytes4(dst, 0x01);
  int2byte(dst + 4, 0x02);
  return 5;
}

static size_t message_notinterested(char *dst) {
  int2bytes4(dst, 0x01);
  int2byte(dst + 4, 0x03);
  return 5;
}

static size_t message_have(char *dst, uint32_t index) {
  int2bytes4(dst, 0x05);
  int2byte(dst + 4, 0x04);
  int2bytes4(dst + 5, index);
  return 9;
}

static size_t message_bitfield(char *dst, bitmap *bm) {
  int2bytes4(dst, bm->len + 1);
  int2byte(dst + 4, 0x05);
  memcpy(dst + 5, bm->buf, bm->len);
  return 5 + bm->len;
}

static size_t message_request(char *dst, uint32_t index, uint32_t begin, uint32_t length) {
  int2bytes4(dst, 0x13);
  int2byte(dst + 4, 0x06);
  int2bytes4(dst + 5, index);
  int2bytes4(dst + 9, begin);
  int2bytes4(dst + 13, length);
  return 17;
}

static size_t message_piece(char *dst, uint32_t index, uint32_t begin, char* block) {
  int2bytes4(dst, BT_PIECE_BLOCK_LEN + 0x09);
  int2byte(dst + 4, 0x07);
  int2bytes4(dst + 5, index);
  int2bytes4(dst + 9, begin);
  memcpy(dst + 13, block, BT_PIECE_BLOCK_LEN);
  return 13 + BT_PIECE_BLOCK_LEN;
}

static size_t message_cancel(char *dst, uint32_t index, uint32_t begin, uint32_t length) {
  int2bytes4(dst, 0x13);
  int2byte(dst + 4, 0x08);
  int2bytes4(dst + 5, index);
  int2bytes4(dst + 9, begin);
  int2bytes4(dst + 13, length);
  return 17;
}

