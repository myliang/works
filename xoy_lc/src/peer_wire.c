#include "peer_wire.h"

#include <string.h>
#include "config.h"

// 16K
#define PIECE_BLOCK_LEN 0x4000

static char bittorrent_protocol[] = "BitTorrent protocol";

void b_peer_wire_handshake(char *dst, const char *info_hash, const char *peer_id)  {
  dst[0] = sizeof(bittorrent_protocol) - 1;
  memcpy(dst + 1, bittorrent_protocol, sizeof(bittorrent_protocol) - 1);
  memcpy(dst + 20, "00000000", 8);
  memcpy(dst + 28, info_hash, 20);
  memcpy(dst + 48, peer_id, 20);
}


void b_peer_wire_keepalive(char *dst) {
  int2bytes4(dst, 0x00);
}

void b_peer_wire_choke(char *dst) {
  int2bytes4(dst, 0x01);
  int2byte(dst + 4, 0x00);
}

void b_peer_wire_unchoke(char *dst) {
  int2bytes4(dst, 0x01);
  int2byte(dst + 4, 0x01);
}

void b_peer_wire_interested(char *dst) {
  int2bytes4(dst, 0x01);
  int2byte(dst + 4, 0x02);
}

void b_peer_wire_notinterested(char *dst) {
  int2bytes4(dst, 0x01);
  int2byte(dst + 4, 0x03);
}

void b_peer_wire_have(char *dst, uint32_t index) {
  int2bytes4(dst, 0x05);
  int2byte(dst + 4, 0x04);
  int2bytes4(dst + 5, index);
}

void b_peer_wire_bitfield(char *dst, bitmap *bm) {
  int2bytes4(dst, bm->len + 1);
  int2byte(dst + 4, 0x05);
  memcpy(dst + 5, bm->buf, bm->len);
}

void b_peer_wire_request(char *dst, uint32_t index, uint32_t begin, uint32_t length) {
  int2bytes4(dst, 0x13);
  int2byte(dst + 4, 0x06);
  int2bytes4(dst + 5, index);
  int2bytes4(dst + 9, begin);
  int2bytes4(dst + 13, length);
}

void b_peer_wire_piece(char *dst, uint32_t index, uint32_t begin, char* block) {
  int2bytes4(dst, PIECE_BLOCK_LEN + 0x09);
  int2byte(dst + 4, 0x07);
  int2bytes4(dst + 5, index);
  int2bytes4(dst + 9, begin);
  memcpy(dst + 13, block, PIECE_BLOCK_LEN);
}

void b_peer_wire_cancel(char *dst, uint32_t index, uint32_t begin, uint32_t length) {
  int2bytes4(dst, 0x13);
  int2byte(dst + 4, 0x08);
  int2bytes4(dst + 5, index);
  int2bytes4(dst + 9, begin);
  int2bytes4(dst + 13, length);
}

