#include "bitmap.h"
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define BYTE_BITS 8
#define BIT_MASK 0x01
#define ARRAY_SIZE(ints, type) sizeof(ints)/sizeof(type)
#define BYTE_BIT_INDEX(index) {index/BYTE_BITS, index%BYTE_BITS}

static int bit_masks[] = {0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01};

bitmap* bitmap_new(int len) {
  bitmap* bm = malloc(sizeof(bitmap));
  bm->buf = malloc(len);
  bzero(bm->buf, len);
  bm->len = len;
  return bm;
}

bitmap* bitmap_init(const char *buf, int len) {
  bitmap *bm = bitmap_new(len);
  memcpy(bm->buf, buf, len);
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
  uint64_t len = 0;
  int i, j;
  int bit_masks_len = ARRAY_SIZE(bit_masks, int);
  for (i = 0; i < bm->len; i++) {
    char c = bm->buf[i];
    for (j = 0; j < bit_masks_len; j++) {
      if ((c & bit_masks[j]) != 0)
        len++;
    }
  }

  return len;
}

int bitmap_get(bitmap* bm, uint64_t index) {
  int bb[] = BYTE_BIT_INDEX(index);
  return (bm->buf[bb[0]] >> (BYTE_BITS - bb[1])) & BIT_MASK;
}

void bitmap_set(bitmap* bm, uint64_t index) {
  int bb[] = BYTE_BIT_INDEX(index);
  int mask = BIT_MASK << (BYTE_BITS - bb[1]);
  bm->buf[bb[0]] |= mask;
}

void bitmap_clear(bitmap* bm, uint64_t index) {
  int bb[] = BYTE_BIT_INDEX(index);
  int mask = BIT_MASK << (BYTE_BITS - bb[1]);
  bm->buf[bb[0]] &= ~mask;
}

void bitmap_print(bitmap* bm) {
  int i;
  for (i = 0; i < bm->len; i++) {
    unsigned char c = bm->buf[i];
    printf("%.2x ", c);
  }
  printf("\n");
}

void bitmap_compare(int ret[], bitmap* bm1, bitmap* bm2) {
  int i, j;
  int bit_masks_len = ARRAY_SIZE(bit_masks, int);
  for (i = 0; i < bm1->len; i++) {
    for (j = 0; j < bit_masks_len; j++) {
      int r1 = bm1->buf[i] & bit_masks[j];
      int r2 = bm2->buf[i] & bit_masks[j];
      if (r1 == r2) continue ;
      if (r1 == 1) ret[0]++;
      else ret[1]++;
    }
  }
}
