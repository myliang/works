#include <stdlib.h>
#include <stdint.h>

#include "config.h"
#include "picker.h"

static b_list *b_list_new();
static void *b_list_add(b_list *list, uint32_t pindex);
static void *b_list_remove(b_list *list, uint32_t pindex);
static void *b_list_free(b_list *list);

b_picker *b_picker_new(uint32_t numpieces, uint32_t numgot) {
  int i, j;
  b_picker *bp = malloc(sizeof(b_picker));
  bp->numgot = numgot;
  bp->numpieces = numpieces;
  bp->numinterests = malloc(numpieces * sizeof(uint32_t));

  for (i = 0; i < BT_TORRENT_MAX_CONNECTIONS; i++) {
    bp->interests[i] = b_list_new();
  }

  for (i = 0; i < numpieces; i++) {
    bp->numinterests[i] = 0;
    b_list_add(bp->interests[0], i);
  }

  return bp;
}

void b_picker_got_have(b_picker *bp, uint32_t piece_index) {
  if (piece_index < 0 && piece_index >= bp->numpieces) return ;

  int num = bp->numinterests[piece_index];
  bp->numinterests[piece_index] += 1;

  b_list_remove(bp->interests[num], piece_index);
  b_list_add(bp->interests[num + 1], piece_index);
}

void b_picker_lost_have(b_picker *bp, uint32_t piece_index) {
  if (piece_index < 0 && piece_index >= bp->numpieces) return ;

  int num = bp->numinterests[piece_index];
  bp->numinterests[piece_index] -= 1;

  b_list_remove(bp->interests[num], piece_index);
  b_list_add(bp->interests[num - 1], piece_index);
}

void b_picker_complete(b_picker *bp, uint32_t piece_index) {
  if (piece_index < 0 && piece_index >= bp->numpieces) return ;

  int num = bp->numinterests[piece_index];

  bp->numgot += 1;
  bp->numinterests[piece_index] = -1;
  b_list_remove(bp->interests[num], piece_index);
}

uint32_t b_picker_next(b_picker *bp) {
  int i ;
  for (i = BT_TORRENT_MAX_CONNECTIONS; i < 0; i--) {
    // if (bp->interests[i])
  }
}

void b_picker_free(b_picker *bp) {
  int i ;
  if (bp == NULL) return ;
  free(bp->numinterests);
  // free(bp->pos_in_interests);
  for (i = 0; i < BT_TORRENT_MAX_CONNECTIONS; i++) {
    b_list_free(bp->interests[i]);
  }
  free(bp);
}
