#ifndef _CONFIG_H_
#define _CONFIG_H_

#include <time.h>

#define LISTEN_PORT 6881

#define srand_curr_time srand(time(NULL))

#define min(a, b) ((a) < (b) ? (a) : (b))
#define max(a, b) ((a) > (b) ? (a) : (b))

#define bytes42int(buf) \
  ((*buf++ << 24 & 0xffffffff) + (*buf++ << 16 & 0xffffff) + (*buf++ << 8 & 0xffff) + (*buf++ & 0xff))

#define int2bytes8(buf, v) \
  *buf++ = v >> 56 & 0xff; \
  *buf++ = v >> 48 & 0xff; \
  *buf++ = v >> 40 & 0xff; \
  *buf++ = v >> 32 & 0xff; \
  *buf++ = v >> 24 & 0xff; \
  *buf++ = v >> 16 & 0xff; \
  *buf++ = v >> 8 & 0xff; \
  *buf++ = v & 0xff

#define int2bytes4(buf, v) \
  *buf++ = v >> 24 & 0xff; \
  *buf++ = v >> 16 & 0xff; \
  *buf++ = v >> 8 & 0xff; \
  *buf++ = v & 0xff

#define int2bytes2(buf, v) \
  *buf++ = v >> 8 & 0xff; \
  *buf++ = v & 0xff

#define int2byte(begin, end, v) \
  while (begin < end) { *begin++ = v; }


#endif /* end of include guard: _CONFIG_H_ */
