#ifndef _PEER_WIRE_H_
#define _PEER_WIRE_H_

#include <stdint.h>
#include "bitmap.h"

// handshake : <pstrlen><pstr><reserved><info_hash><peer_id>
void b_peer_wire_handshake(char *dst, const char *info_hash, const char *peer_id);

// keep-alive: <len=0000>
void b_peer_wire_keepalive(char *dst);
// choke: <len=0001><id=0>
void b_peer_wire_choke(char *dst);
// unchoke: <len=0001><id=1>
void b_peer_wire_unchoke(char *dst);
// interested: <len=0001><id=2>
void b_peer_wire_interested(char *dst);
// not interested: <len=0001><id=3>
void b_peer_wire_notinterested(char *dst);
// have: <len=0005><id=4><piece index>
void b_peer_wire_have(char *dst, uint32_t index);
// bitfield: <len=0001+X><id=5><bitfield>
// X : bitfield len
void b_peer_wire_bitfield(char *dst, bitmap *bm);
// request: <len=0013><id=6><index><begin><length>
// index: piece index
// begin: offset of piece
// length = 16KB
void b_peer_wire_request(char *dst, uint32_t index, uint32_t begin, uint32_t length);
// piece: <len=0009+X><id=7><index><begin><block>
// X : block len , default 16KB
void b_peer_wire_piece(char *dst, uint32_t index, uint32_t begin, char* block);
// cancel: <len=0013><id=8><index><begin><length>
void b_peer_wire_cancel(char *dst, uint32_t index, uint32_t begin, uint32_t length);
// port: <len=0003><id=9><listen-port>
// void b_peer_wire_port(char *dst, uint16_t port);

#endif /* end of include guard: _PEER_WIRE_H_ */
