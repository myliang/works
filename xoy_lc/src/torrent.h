#ifndef _TORRENT_H_
#define _TORRENT_H_

#include <sys/types.h>
#include "bencode.h"
#include "peer.h"

typedef struct b_torrent_tracker{
  char* url;
  struct b_torrent_tracker* next;
} b_torrent_tracker;

typedef struct b_torrent_file{
  int64_t size;
  char* name;
  struct b_torrent_file* next;
} b_torrent_file;

typedef struct b_torrent{
  char* name;
  char* comment;
  char* created_by;
  char* encoding;
  char* pieces;
  int64_t create_date;
  int64_t piece_size;
  unsigned char info_hash[21];
  unsigned char peer_id[20];

  b_torrent_tracker* tracker;
  b_torrent_file* file;

  b_peer* peer;
} b_torrent;

b_torrent* b_torrent_init(b_encode* bp);
void b_torrent_print(b_torrent* btp);

#endif /* end of include guard: _TORRENT_H_ */
