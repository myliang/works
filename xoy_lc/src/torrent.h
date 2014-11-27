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
  uint32_t index;
  uint32_t begin;
  int fd;

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
  char* file_path; // file storage path

  // upload, download and left bytes
  uint64_t uploaded;
  uint64_t downloaded;
  uint64_t left;

  bitmap *bitfield;

  b_peer* peer;
  int peer_len;

  struct b_torrent *next;

} b_torrent;

// init and print methods
b_torrent* b_torrent_init(b_encode* bp);
void b_torrent_print(b_torrent* btp);

// store torrent struct information and file data
void b_torrent_store_all(b_peer *bp, b_torrent *bt);

// store and recover methods
void b_torrent_store(const char* filename, b_torrent* bt);
b_torrent* b_torrent_recover(const char* filename);

// store real data file
void b_torrent_file_read(b_peer_request *req, char *dst, b_torrent *bt);
void b_torrent_file_write(b_peer_response *res, b_torrent *bt);

#endif /* end of include guard: _TORRENT_H_ */

