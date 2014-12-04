#ifndef _B_PICKER_H_
#define _B_PICKER_H_

typedef struct b_node {
  uint32_t value;
  struct b_node *next;
} b_node;

typedef struct b_list {
  struct b_node *head;
  struct b_node *tail;
  int len;
} b_list;

typedef struct b_picker{
  uint32_t numpieces;
  uint32_t numgot;

  b_list* interests[BT_TORRENT_MAX_CONNECTIONS]; // [[][][]] received piece num => pieces
  // uint32_t *pos_in_interests; // piece in interests []
  uint32_t *numinterests; // each piece num []

} b_picker;

b_picker *b_picker_new(uint32_t numpieces, uint32_t numgot);
void b_picker_got_have(b_picker *bp, uint32_t piece_index);
void b_picker_lost_have(b_picker *bp, uint32_t piece_index);
void b_picker_complete(b_picker *bp, uint32_t piece_index);
uint32_t b_picker_next(b_picker *bp);
void b_picker_free(b_picker *bp);



#endif /* end of include guard: _B_PICKER_H_ */
