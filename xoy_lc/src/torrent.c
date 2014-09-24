#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <sys/types.h>

#include "sha1.h"
#include "torrent.h"

#define DEBUG
#define min(a, b) ((a) < (b) ? (a) : (b))
#define max(a, b) ((a) > (b) ? (a) : (b))

// fwrite
#define FWRITE_KV(v, len, fp) \
  fwrite(&len, 4, 1, fp); \
  fwrite(v, len, 1, fp)
#define FWRITE_KV_STRING(v, fp) \
  len = strlen(v); \
  FWRITE_KV(v, len, fp)
#define FWRITE_KV_SIZE_OF(v, fp) \
  len = sizeof(v); \
  FWRITE_KV(&v, len, fp)

// fread
#define FREAD_KV_STRING(k, len, fp) \
  fread(&len, 4, 1, fp); \
  k = malloc(len + 1); \
  fread(k, len, 1, fp); \
  k[len] = '\0'
#define FREAD_KV_SIZE_OF(k, len, fp) \
  fread(&len, 4, 1, fp); \
  fread(&k, len, 1, fp)

static b_torrent_tracker* malloc_tracker(char* src, int len);
static char* malloc_string(char* src, int len);
static void _b_torrent_init(b_torrent* tt, b_encode* bp);
static b_torrent_file* malloc_file(char* str, int strlen, int64_t file_size);

b_torrent* b_torrent_init(b_encode* bp) {
  b_torrent* tt = malloc(sizeof(b_torrent));
  _b_torrent_init(tt, bp);
  return tt;
}

void b_torrent_print(b_torrent* btp) {
  if (NULL != btp) {
    printf("torrent: \n");
    printf("    name: %s\n", btp->name);
    printf("    created by: %s\n", btp->created_by);
    printf("    encoding: %s\n", btp->encoding);
    printf("    create date: %llu\n", (unsigned long long)btp->create_date);
    printf("    piece size: %llu\n", (unsigned long long)btp->piece_size);
    printf("    peer id: %s\n", btp->peer_id);
    printf("    total_size: %llu\n", (unsigned long long)btp->total_size);
    printf("    uploaded: %llu\n", (unsigned long long)btp->uploaded);
    printf("    downloaded: %llu\n", (unsigned long long)btp->downloaded);
    printf("    left: %llu\n", (unsigned long long)btp->left);
    printf("    info_hash: ");
    int i = 0;
    for (i = 0; i < 20; i++) {
      printf("%.2x", btp->info_hash[i]);
    }
    printf("\n");
    printf("    trackers: %d\n", btp->tracker_len);
    b_torrent_tracker* tracker = btp->tracker;
    while (tracker != NULL) {
      printf("      %s\n", tracker->url);
      tracker = tracker->next;
    }

    printf("    files: %d\n", btp->file_len);
    b_torrent_file* file = btp->file;
    while (file != NULL) {
      printf("      %s[%lld]\n", file->name, (long long)file->size);
      file = file->next;
    }
    printf("    comment: %s\n", btp->comment);
  }
}

void b_torrent_store(const char* filename, b_torrent* bt) {
  FILE* fp = fopen(filename, "wb");
  if (fp == NULL) {
    fprintf(stderr, "%s:%d can not open file[%s]\n", __FILE__, __LINE__, filename);
    return ;
  }

  size_t len = 0;
  FWRITE_KV_STRING(bt->name, fp);
  FWRITE_KV_STRING(bt->comment, fp);
  FWRITE_KV_STRING(bt->created_by, fp);
  FWRITE_KV_STRING(bt->encoding, fp);
  FWRITE_KV_STRING(bt->pieces, fp);

  FWRITE_KV_SIZE_OF(bt->create_date, fp);
  FWRITE_KV_SIZE_OF(bt->piece_size, fp);
  FWRITE_KV_SIZE_OF(bt->total_size, fp);
  FWRITE_KV_SIZE_OF(bt->info_hash, fp);
  FWRITE_KV_SIZE_OF(bt->peer_id, fp);
  FWRITE_KV_SIZE_OF(bt->uploaded, fp);
  FWRITE_KV_SIZE_OF(bt->downloaded, fp);
  FWRITE_KV_SIZE_OF(bt->left, fp);

  // tracker
  b_torrent_tracker* tracker = bt->tracker;
  while (tracker != NULL) {
    FWRITE_KV_STRING(tracker->url, fp);
    tracker = tracker->next;
  }

  // files
  b_torrent_file* file = bt->file;
  while (file != NULL) {
    FWRITE_KV_STRING(file->name, fp);
    FWRITE_KV_SIZE_OF(file->size, fp);
    file = file->next;
  }

  fclose(fp);
}

b_torrent* b_torrent_recover(const char* filename) {
  b_torrent* bt = malloc(sizeof(b_torrent));

  FILE* fp = fopen(filename, "rb");
  if (fp == NULL) {
    fprintf(stderr, "%s:%d can not open file[%s]\n", __FILE__, __LINE__, filename);
    return NULL;
  }

  size_t len;
  FREAD_KV_STRING(bt->name, len, fp);
  FREAD_KV_STRING(bt->comment, len, fp);
  FREAD_KV_STRING(bt->created_by, len, fp);
  FREAD_KV_STRING(bt->encoding, len, fp);
  FREAD_KV_STRING(bt->pieces, len, fp);

  FREAD_KV_SIZE_OF(bt->create_date, len, fp);
  FREAD_KV_SIZE_OF(bt->piece_size, len, fp);
  FREAD_KV_SIZE_OF(bt->total_size, len, fp);
  FREAD_KV_SIZE_OF(bt->info_hash, len, fp);
  FREAD_KV_SIZE_OF(bt->peer_id, len, fp);
  FREAD_KV_SIZE_OF(bt->uploaded, len, fp);
  FREAD_KV_SIZE_OF(bt->downloaded, len, fp);
  FREAD_KV_SIZE_OF(bt->left, len, fp);

  fclose(fp);
  return bt;
}

static void _b_torrent_init (b_torrent* tt, b_encode* bp) {
  b_dict* bd = bp->data.dpv;
  char buf[1024];
  // printf("type=%d\n", bp->type);
  while(NULL != bd) {
    // printf("key=%s\n", bd->key);
    unsigned int key_len = strlen(bd->key);
    if(strncmp("announce-list", bd->key, max(key_len, 13)) == 0) {
      b_list* bl = bd->value->data.lpv;
      b_torrent_tracker head;
      b_torrent_tracker* tp = &head;
      tt->tracker_len = 0;
      while(NULL != bl) {
        b_encode* bll = bl->item->data.lpv->item;
        tp = tp->next = malloc_tracker(bll->data.cpv, bll->len);
        // printf("%s\n", tp->url);
        bl = bl->next;
        tt->tracker_len++;
      }
      tt->tracker = head.next;
    }
    if(strncmp("announce", bd->key, max(key_len, 8)) == 0) {
      tt->tracker_len = 0;
      if (NULL == tt->tracker) {
        b_encode* bp = bd->value;
        tt->tracker = malloc_tracker(bp->data.cpv, bp->len);
        tt->tracker_len = 1;
      }
    }

    if(strncmp("comment", bd->key, max(key_len, 7)) == 0) {
      tt->comment = malloc_string(bd->value->data.cpv, bd->value->len);
    }
    if(strncmp("encoding", bd->key, max(key_len, 8)) == 0) {
      tt->encoding = malloc_string(bd->value->data.cpv, bd->value->len);
    }
    if(strncmp("created by", bd->key, max(key_len, 10)) == 0) {
      tt->created_by = malloc_string(bd->value->data.cpv, bd->value->len);
    }

    if(strncmp("creation date", bd->key, max(key_len, 13)) == 0) {
      tt->create_date = bd->value->data.iv;
    }

    if (strncmp("length", bd->key, max(key_len, 6)) == 0) {
      tt->total_size = bd->value->data.iv;
    }

    if (strncmp("name", bd->key, max(key_len, 4)) == 0) {
      tt->name = malloc_string(bd->value->data.cpv, bd->value->len);
    } else if (strncmp("pieces", bd->key, max(key_len, 6)) == 0) {
      tt->pieces = malloc_string(bd->value->data.cpv, bd->value->len);
    } else if (strncmp("piece length", bd->key, max(key_len, 12)) == 0) {
      tt->piece_size = bd->value->data.iv;
    } else if (strncmp("files", bd->key, max(key_len, 5)) == 0) {
      b_list* list = bd->value->data.lpv;
      b_torrent_file head;
      b_torrent_file* tf = &head;
      tt->file_len = 0;
      while (list) {
        b_dict* ldict = list->item->data.dpv;
        int64_t size = ldict->value->data.iv;
        b_list* paths = ldict->next->value->data.lpv;
        unsigned int len = 0;
        while (NULL != paths) {
          memcpy(buf + len, paths->item->data.cpv, paths->item->len);
          len += paths->item->len;
          paths = paths->next;
        }
        buf[len] = '\0';
        // printf("file buffer: %s\n", buf);
        tf = tf->next = malloc_file(buf, len, size);
        list = list->next;

        // set total size
        tt->total_size += size;
        tt->file_len++;
        // printf("%llu\n", (unsigned long long)tt->total_size);
      }
      tt->file = head.next;
    } else if (strncmp("info", bd->key, max(key_len, 4)) == 0) {
      _b_torrent_init(tt, bd->value);
      SHA1_CTX context;
      SHA1Init(&context);
      SHA1Update(&context, (unsigned char*)&bd->value->begin[0], bd->value->len);
      SHA1Final(tt->info_hash, &context);
      // tt->info_hash[20] = '\0';
    }
    bd = bd->next;
  }

  // set peer_id
  srand(time(NULL));
  sprintf((char*)tt->peer_id, "-XOY1000-%d", rand());
}

static b_torrent_file* malloc_file(char* str, int strlen, int64_t file_size) {
  b_torrent_file* tf = malloc(sizeof(b_torrent_file));
  tf->size = file_size;
  tf->name = malloc_string(str, strlen);
  return tf;
}
static b_torrent_tracker* malloc_tracker(char* src, int len) {
  b_torrent_tracker* ttk = malloc(sizeof(b_torrent_tracker));
  ttk->next = NULL;
  ttk->url = malloc_string(src, len);
  return ttk;
}
static char* malloc_string (char* src, int len) {
  char* dst = malloc(len + 1);
  memcpy(dst, src, len);
  dst[len] = '\0';
  return dst;
}
