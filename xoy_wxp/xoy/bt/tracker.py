#!/bin/python
# -*- coding:utf-8 -*-

# http:
#   request:
#   response:

import threading
import requests
import time
import socket
import struct

from xoy import byte_helper, config
from bencode import BEncode
from udp_tracker import UdpTracker

CONNECTION_TIME_OUT = int(config.bt.connection_time_out)

TRACKER_EVENT = ["started", "completed", "stopped"]
tracker_list = []

class Tracker:
  def __init__(self, torrent, peer_id, port, uploaded, downloaded, left):
    self.torrent = torrent
    self.peer_id = peer_id
    self.port = port
    self.uploaded = uploaded
    self.downloaded = downloaded
    self.left = left
    self.event = TRACKER_EVENT[0]
    self.compact = 1
    self.numwant = 50

    # (ip, port)
    self.peers = set()
    self.running = False

    # print "tracker :::::"
    tracker_list.append(self)

  def peers_ip_port(self):
    return self.peers

  def start(self):
    pass
    # if not self.running:
    #   self.running = True
    #   self.loop = Thread(target = self.__loop_peers)
    #   self.loop.start()

  def stop(self):
    pass
    # if self.running:
    #   self.running = False
    #   self.loop.join()

  def loop(self):
    for url in self.torrent.announces:
      print "tracker_url=", url
      if url.startswith('http'):
        self.__http_request(url)
      elif url.startswith('udp'):
        self.__udp_request(url)
      else:
        print "tracker_url [%s] is not available" % url


  def __udp_request(self, url):
    ip_port = to_ip_port_from_url(url)
    ipports = UdpTracker(ip_port).send(self.torrent.info_hash, self.peer_id,
        self.downloaded, self.left, self.uploaded, self.event, self.numwant, self.port)
    if ipports:
      for ipport in ipports:
        self.peers.add(ipport)

  def __http_request(self, url):
    try:
      payload = {'info_hash': self.torrent.info_hash, 'peer_id': self.peer_id,
          'port': self.port, 'uploaded': self.uploaded, 'downloaded': self.downloaded,
          'left': self.left, 'event': self.event, 'compact': self.compact, 'numwant': self.compact}
      req = requests.get(url, params = payload, timeout = CONNECTION_TIME_OUT)
      print "req.url = ", req.url

      # print "req : ", req
      res = req.content
      # print "res type = ", type(res)
      # print "tracker response = ", res
      res = BEncode(res).parse()
      print res
      if res.get('peers'):
        bpeers = res.get('peers')
        blen = len(bpeers)
        i = 0
        while i < blen:
          ip, port = struct.unpack('lH', bpeers[i: i + 6])
          # ip = byte_helper.bytes42int(bpeers[i: i + 4])
          print "ip before = ", ip, port
          ip = socket.ntoa(struct.pack('l', socket.inet_htol(ip)))
          print ip
          ip_port = (socket.inet_ntoa(bpeers[i: i + 4]), byte_helper.bytes22int(bpeers[i + 4: i + 6]))
          print "peer ip port = ", ip_port
          self.peers.add(ip_port)
          i += 6
    except Exception, e:
      print "Exception", e


# parse_ip_port_by_url
def to_ip_port_from_url(url):
  port = 80
  urls = url.split(':')
  ip = urls[1].replace('//', '').split('/')[0]
  if len(urls) > 2:
    port = int(urls[-1].split('/')[0])

  return (ip, port)


# tracker_list loop
# a threading run
def loop_tracker_list():
  while True:
    # print len(tracker_list)
    for tracker in tracker_list:
      # print "loop"
      tracker.loop()

    time.sleep(int(config.bt.tracker_interval_time))

threading.Thread(target = loop_tracker_list).start()


