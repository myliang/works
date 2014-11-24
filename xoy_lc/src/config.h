#ifndef _CONFIG_H_
#define _CONFIG_H_

#include <time.h>

#define srand_curr_time srand(time(NULL))

#define min(a, b) ((a) < (b) ? (a) : (b))
#define max(a, b) ((a) > (b) ? (a) : (b))

#define bytes42int(buf) \
  (((buf)[0] << 24 & 0xffffffff) + ((buf)[1] << 16 & 0xffffff) + ((buf)[2] << 8 & 0xffff) + ((buf)[3] & 0xff))

#define bytes22int(buf) \
  (((buf)[0] << 8 & 0xffff) + ((buf)[1] & 0xff))

#define int2bytes8(buf, v) \
  (buf)[0] = (v) >> 56 & 0xff; \
  (buf)[1] = (v) >> 48 & 0xff; \
  (buf)[2] = (v) >> 40 & 0xff; \
  (buf)[3] = (v) >> 32 & 0xff; \
  (buf)[4] = (v) >> 24 & 0xff; \
  (buf)[5] = (v) >> 16 & 0xff; \
  (buf)[6] = (v) >> 8 & 0xff; \
  (buf)[7] = (v) & 0xff

#define int2bytes4(buf, v) \
  (buf)[0] = (v) >> 24 & 0xff; \
  (buf)[1] = (v) >> 16 & 0xff; \
  (buf)[2] = (v) >> 8 & 0xff; \
  (buf)[3] = (v) & 0xff

#define int2bytes2(buf, v) \
  (buf)[0] = (v) >> 8 & 0xff; \
  (buf)[1] = (v) & 0xff

#define int2byte(buf, v) \
  (buf)[0] = (v) & 0xff

// bittorrent
#define BT_LISTEN_PORT "6881"
// max connections num of torrent
#define BT_TORRENT_MAX_CONNECTIONS 10
// 16K
#define BT_PIECE_BLOCK_LEN 0x4000
// protocol
static char bittorrent_protocol[] = "BitTorrent protocol";

#endif /* end of include guard: _CONFIG_H_ */
