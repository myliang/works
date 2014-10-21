#ifndef _PEER_WIRE_H_
#define _PEER_WIRE_H_

#include <stdint.h>
#include <sys/types.h>

#include "torrent.h"
#include "bitmap.h"

// message change download or upload
void b_peer_wire_message(b_torrent *bt);

int b_peer_wire_send_message(b_peer* bp, b_torrent *bt);
int b_peer_wire_recv_message(b_peer* bp, const char *buf, b_torrent *bt);

#endif /* end of include guard: _PEER_WIRE_H_ */
