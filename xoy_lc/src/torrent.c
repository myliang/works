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

static b_torrent_tracker* malloc_tracker(char* src, int len);
static char* malloc_string(char* src, int len);
static void _b_torrent_init(b_torrent* tt, b_encode* bp);
static b_torrent_file* malloc_file(char* str, int strlen, int64_t file_size);

b_torrent* b_torrent_init(b_encode* bp) {
  b_torrent* tt = malloc(sizeof(b_torrent));
  _b_torrent_init(tt, bp);
  return tt;
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
      while(NULL != bl) {
        b_encode* bll = bl->item->data.lpv->item;
        tp = tp->next = malloc_tracker(bll->data.cpv, bll->len);
        printf("%s\n", tp->url);
        bl = bl->next;
      }
      tt->tracker = head.next;
    }
    if(strncmp("announce", bd->key, max(key_len, 8)) == 0) {
      if (NULL == tt->tracker) {
        b_encode* bp = bd->value;
        tt->tracker = malloc_tracker(bp->data.cpv, bp->len);
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

void b_torrent_print(b_torrent* btp) {
  if (NULL != btp) {
    printf("torrent: \n");
    printf("    name: %s\n", btp->name);
    printf("    created by: %s\n", btp->created_by);
    printf("    encoding: %s\n", btp->encoding);
    printf("    create date: %lld\n", (long long)btp->create_date);
    printf("    piece size: %lld\n", (long long)btp->piece_size);
    printf("    peer id: %s\n", btp->peer_id);
    printf("    ");
    int i = 0;
    for (i = 0; i < 20; i++) {
      printf("%.2x", btp->info_hash[i]);
    }
    printf("\n");
    printf("    trackers:\n");
    b_torrent_tracker* tracker = btp->tracker;
    while (tracker != NULL) {
      printf("      %s\n", tracker->url);
      tracker = tracker->next;
    }

    printf("    files:\n");
    b_torrent_file* file = btp->file;
    while (file != NULL) {
      printf("      %s[%lld]\n", file->name, (long long)file->size);
      file = file->next;
    }
    printf("    comment: %s\n", btp->comment);
  }
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
