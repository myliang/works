#!/bin/python
# coding = utf-8

TRACKER_EVENT = ["started", "completed", "stopped"]

class Tracker:
  def __init__(self, torrent, peer_id, port, uploaded, downloaded, left):
    self.torrent = torrent
    self.peer_id = peer_id
    self.port = port
    self.uploaded = uploaded
    self.downloaded = downloaded
    self.left = left
    self.event = TRACKER_EVENT[0]

  def request_urls(self):
    urls = []
    for url in self.torrent.announces:
      urls.append("%s?info_hash=%s&peer_id=%s&port=%d&uploaded=%d&downloaded=%d&left=%d&event=%s" % (url,
          self.torrent.info_hash, self.peer_id, self.port, self.uploaded, self.downloaded,
          self.left, self.event))
    return urls


