#ifndef _BITMAP_H_
#define _BITMAP_H_

#include <stdint.h>

typedef struct {
  char *buf;
  int len;
} bitmap;

// init , free
bitmap* bitmap_init(int len);
void bitmap_free(bitmap* bm);

// operator
uint64_t bitmap_isseted_count(bitmap* bm);
void bitmap_set(bitmap* bm, uint64_t index);
void bitmap_clear(bitmap* bm, uint64_t index);
void bitmap_print(bitmap* bm);

#endif /* end of include guard: _BITMAP_H_ */
