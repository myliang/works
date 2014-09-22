#include <stdlib.h>
#include <stdio.h>

#include "config.h"
#include "bencode.h"
#include "torrent.h"
#include "tracker.h"
#include "io_http.h"

#define TRACKER_NUMWANT 200

static char *tracker_events[] = {
  "started",
  "stopped",
  "completed"
};

static void request_trackers_with_http(const char* url, b_torrent* tptr, b_peer* pptr, int timeout);
static void request_trackers_with_udp(const char* url, b_torrent* tptr, b_peer* pptr, int timeout);

void request_trackers(b_torrent* tptr, b_peer* pptr, int timeout) {
  b_torrent_tracker* tracker = tptr->tracker;
  while (tracker != NULL) {
    const char *url = tracker->url;
    printf("tracker url:%s\n", url);
    if (url[0] == 'h') { // http tracker
      request_trackers_with_http(url, tptr, pptr, timeout);
    } else if(url[0] == 'u') { // udp tracker
      request_trackers_with_udp(url, tptr, pptr, timeout);
    }
    tracker = tracker->next;
  }
}

static void request_trackers_with_http(const char* url, b_torrent* tptr, b_peer* pptr, int timeout) {
  char full_url[1024];
  snprintf(full_url, sizeof(full_url) - 1,
      "%s?info_hash=%s&peer_id=%s&port=%d&uploaded=%d&downloaded=%d&left=%d&event=%s&numwant=%d&compact=1",
      url, http_uri_hex(tptr->info_hash, 20), http_uri_hex(tptr->peer_id, 20),
      LISTEN_PORT, 0, 0, 10240000, tracker_events[0], TRACKER_NUMWANT);
  full_url[sizeof(full_url) - 1] = '\0';
  printf("url=%s\n", full_url);

  io_http_res* res = http_get(full_url, timeout);
  if (res == NULL) {
    return ;
  }

  // format response content
  b_buffer* buf = b_buffer_init_with_string(res->content, res->content_length);
  b_encode* be = b_encode_init(buf);

  b_encode_print(be);

  b_encode_free(be, buf);

}
static void request_trackers_with_udp(const char* url, b_torrent* tptr, b_peer* pptr, int timeout) {
  //
}
