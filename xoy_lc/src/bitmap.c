#include "bitmap.h"
#include <stdlib.h>
#include <stdint.h>

#define BYTE_BITS 8
#define BIT_MASK 0x01
#define ARRAY_SIZE(ints, type) sizeof(ints)/sizeof(type)
#define BYTE_BIT_INDEX(index) {index/BYTE_BITS, index%BYTE_BITS};

static int bit_masks[] = {0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01};

bitmap* bitmap_init(int len) {
  bitmap* bm = malloc(sizeof(bitmap));
  bm->buf = malloc(len);
  bm->len = len;
  return bm;
}

void bitmap_free(bitmap* bm) {
  if (bm != NULL) {
    if (bm->buf != NULL) {
      free(bm->buf);
    }
    free(bm);
  }
}

uint64_t bitmap_isseted_count(bitmap* bm) {
  uint64_t len;
  int i, j;
  for (i = 0; i < bm->len; i++) {
    char c = bm->buf[i];
    for (j = 0; j < ARRAY_SIZE(bit_masks, int); j++) {
      if (c & bit_masks[j] != 0)
        len++;
    }
  }

  return len;
}


