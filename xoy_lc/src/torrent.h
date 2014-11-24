#ifndef _TORRENT_H_
#define _TORRENT_H_

#include <sys/types.h>
#include <stdint.h>
#include "bencode.h"
#include "peer.h"

typedef struct b_torrent_tracker{
  char* url;
  struct b_torrent_tracker* next;
} b_torrent_tracker;

typedef struct b_torrent_file{
  uint64_t size;
  char* name;
  uint64_t index;
  uint32_t begin;
  struct b_torrent_file* next;
} b_torrent_file;

typedef struct b_torrent{
  char* name;
  char* comment;
  char* created_by;
  char* encoding;
  char* pieces;
  uint64_t create_date;
  uint64_t piece_size; // bytes of a piece
  uint64_t total_size; // files total bytes

  unsigned char info_hash[20];
  unsigned char peer_id[20];

  b_torrent_tracker* tracker;
  b_torrent_file* file;
  unsigned int tracker_len;
  unsigned int file_len;

  // upload, download and left bytes
  uint64_t uploaded;
  uint64_t downloaded;
  uint64_t left;

  bitmap *bitfield;

  b_peer* peer;
  int peer_len;

} b_torrent;

// init and print methods
b_torrent* b_torrent_init(b_encode* bp);
void b_torrent_print(b_torrent* btp);

// store and recover methods
void b_torrent_store(const char* filename, b_torrent* bt);
b_torrent* b_torrent_recover(const char* filename);

#endif /* end of include guard: _TORRENT_H_ */

