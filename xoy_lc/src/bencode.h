#ifndef _BENCODE_H_
#define _BENCODE_H_
// #define DEBUG

typedef enum {
  B_INTEGER = 1,
  B_STRING,
  B_LIST,
  B_DICT
} b_type;

typedef struct {
  char* head;
  char* tail;
  int len;
  char* index;
} b_buffer;

typedef long long b_size;
struct b_encode;

typedef struct b_list{
  struct b_encode* item;
  struct b_list* next;
} b_list;

typedef struct b_dict{
  char* key;
  struct b_encode* value;
  struct b_dict* next;
} b_dict;

typedef struct b_encode{
  b_type type;
  char* begin;
  unsigned int len;
  union {
    b_size iv;
    char* cpv;
    b_list* lpv;
    b_dict* dpv;
  } data;
} b_encode;

// buffer methods
b_buffer* b_buffer_init(const char* file_name);
b_buffer* b_buffer_init_with_string (const char* string, long len);

// bencode
b_encode* b_encode_init(b_buffer* buf);
void b_encode_print(b_encode* bp);
void b_encode_free(b_encode* bp, b_buffer* buf);

#endif /* end of include guard: _BENCODE_H_ */
