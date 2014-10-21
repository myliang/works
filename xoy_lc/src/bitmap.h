#ifndef _BITMAP_H_
#define _BITMAP_H_

#include <stdint.h>

typedef struct {
  unsigned char *buf;
  int len;
} bitmap;

// init , free
bitmap* bitmap_init(int len);
void bitmap_free(bitmap* bm);

// operator
uint64_t bitmap_isseted_count(bitmap* bm);
int bitmap_get(bitmap* bm, uint64_t index);
void bitmap_set(bitmap* bm, uint64_t index);
void bitmap_clear(bitmap* bm, uint64_t index);
void bitmap_print(bitmap* bm);
void bitmap_compare(int ret[], bitmap* bm1, bitmap* bm2);

#endif /* end of include guard: _BITMAP_H_ */
