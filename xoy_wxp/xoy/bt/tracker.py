#!/bin/python
# coding = utf-8

from threading import Thread
import urllib
import urllib2
import time
import sys


from xoy import byte_helper


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

    # (ip, port)
    self.peers = []
    self.running = False

  def peers_ip_port(self):
    return self.peers

  def start(self):
    if not self.running:
      self.running = True
      self.loop = Thread(target = self.__loop_peers)
      self.loop.start()

  def stop(self):
    if self.running:
      self.running = False
      self.loop.join()

  def __loop_peers(self):
    while self.running:
      time.sleep(10)
      for url in self.__request_urls():
        print "tracker_url=", url
        if url.startswith('http'):
          self.__http_request(url)


  def __http_request(self, url):
    try:
      res = urllib2.urlopen(url, timeout = 10).read()
      print "tracker response = ", res
      if "failure reason" not in res:
        if type(res) == str:
          res_len = len(res)
          i = 0
          while i < res_len:
            ip_port = (socket.inet_ntoa(res[i: i + 4]), byte_helper.bytes22int(res[i + 4: i + 6]))
            self.peers.append(ip_port)
            i += 6
        elif type(res) == list:
          self.peers = [(p["ip"], p["port"]) for p in peers]
    except Exception, e:
      print e

  def __request_urls(self):
    urls = []
    payload = {'info_hash': self.torrent.info_hash, 'peer_id': self.peer_id,
        'port': self.port, 'uploaded': self.uploaded, 'downloaded': self.downloaded,
        'left': self.left, 'event': self.event}
    payload = urllib.urlencode(payload)
    for url in self.torrent.announces:
      urls.append("%s?%s" % (url, payload))
    return urls



