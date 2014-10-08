#ifndef _TRACKER_H_
#define _TRACKER_H_

#include "torrent.h"
#include "peer.h"

// send request to tracker
void request_trackers(b_torrent* tptr, int timeout);

#endif /* end of include guard: _TRACKER_H_ */
