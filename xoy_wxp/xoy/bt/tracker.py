#!/bin/python
# -*- coding:utf-8 -*-

# http:
#   request:
#   response:

import threading
import urllib
import urllib2
import time
import random

import socket
import select

from xoy import byte_helper

CONNECTION_TIME_OUT = 10

# udp request
# req1 : Connecttion_ID(8字节）   0（4字节） Transaction_ID（4字节）
# res1 : 0（4字节） Transaction_ID（4字节） Connecttion_ID(8字节）
# req2 : InfoHash   20字节, PeerID     20字节, DownLoad  8字节, Left   8字节, UpLoad  8字节, Event    4字节, IP       4字节,
#       下面是8个字节的空(88-96字节是空的内容), port      4字节, 然后是可选内容 主要是服务器的HostName, UserName, PassWord
# res2 : 1（4字节） Transaction_ID（4字节） Interval Time(4) 当前下载数目(4) 种子数(4) 然后是IP（4字节）和Port(2字节）)

def getrandint():
  return random.randint(0, 10000)
def udp_req1_bytes():
  req1bytes = bytearray(16)
  req1bytes[0:8] = byte_helper.int2bytes8(0x41727101980)
  req1bytes[8:12] = [0x00, 0x00, 0x00, 0x00]

  transaction_id = getrandint()
  req1bytes[12:16] = byte_helper.int2bytes4(transaction_id)
  return (transaction_id, req1bytes)

def udp_req2_bytes(connection_id, info_hash, peer_id, downloaded, left, uploaded, event, numwant, port):
  bs = bytearray(98)
  bs[0:8] = connection_id
  bs[8:12] = byte_helper.int2bytes4(0x01)
  transaction_id = getrandint()
  bs[12:16] = byte_helper.int2bytes4(transaction_id)
  bs[16:36] = info_hash
  bs[36:56] = peer_id
  bs[56:64] = byte_helper.int2bytes8(downloaded)
  bs[64:72] = byte_helper.int2bytes8(left)
  bs[72:80] = byte_helper.int2bytes8(uploaded)
  bs[80:84] = event # int
  bs[84:88] = byte_helper.int2bytes4(0x00)
  bs[88:92] = byte_helper.int2bytes4(0x00)
  bs[92:96] = byte_helper.int2bytes4(numwant)
  bs[96:98] = byte_helper.int2bytes2(port)
  return (transaction_id, bs)

# parse_ip_port_by_url
def to_ip_port_from_url(url):
  port = 80
  urls = url.split(':')
  ip = urls[1].replace('//', '').split('/')[0]
  if len(urls) > 2:
    port = int(urls[-1].split('/')[0])

  return (ip, port)

class Udp:
  def __init__(self, ip_port):
    self.sockfd = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    print ip_port
    self.sockfd.connect(ip_port)

  def send(self, info_hash, peer_id, donwloaded, left, uploaded, event, numwant, port):
    try:
      print "udp socket connect start..."
      infds, outfds, errfds = select.select([], [self.sockfd], [], CONNECTION_TIME_OUT)
      print "input: ", infds, outfds, errfds
      if len(outfds) == 0:
        print "udp socket timeout"
        return None
      transaction_id, req1bytes = udp_req1_bytes()
      print "udp send message 1 = ", req1bytes
      if self.sockfd.send(req1bytes) != 16:
        print "udp send message error send 16 bytes"
        return None
      infds, outfds, errfds = select.select([self.sockfd], [], [], CONNECTION_TIME_OUT)
      print infds, outfds, errfds
      if len(infds) == 0:
        print "udp socket can't recv"
        return None
      recv_data = self.sockfd.recv(1024)
      print "recv date : ", recv_data
      if recv_data :
        print recv_data
        if byte_helper.bytes42int(recv_data[0:4]) == 0 and byte_helper.bytes42int(recv_data[4:8]) == transaction_id:
          transaction_id, connection_id = recv_data[8:16]
          req2bytes = udp_req2_bytes(connection_id, info_hash, peer_id, downloaded, left, uploaded, event, numwant, port)
          self.sockfd.send(req2bytes)
          recv_data = self.sockfd.recv(4068)
          if recv_data :
            print recv_data
            recv_data_len = len(recv_data)
            if byte_helper.bytes42int(recv_data[0:4]) == 1 and byte_helper.bytes42int(recv_data[4:8]) == transaction_id:
              i = 20
              ipports = []
              while i < recv_data_len:
                ip = socket.inet_ntoa(recv_data[i, i + 4])
                port = recv_data[i + 4, i + 6]
                ipports.append((ip, port))
                i += 6

              return ipports
    except Exception, e:
      print e
    finally:
      self.close()

    return None


  def close(self):
    self.sockfd.close()



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
    ipports = Udp(ip_port).send(self.torrent.info_hash, self.peer_id,
        self.downloaded, self.left, self.uploaded, self.event, self.numwant, self.port)
    if ipports:
      for ipport in ipports:
        self.peers.add(ipport)

  def __http_request(self, url):
    try:
      payload = {'info_hash': self.torrent.info_hash, 'peer_id': self.peer_id,
          'port': self.port, 'uploaded': self.uploaded, 'downloaded': self.downloaded,
          'left': self.left, 'event': self.event, 'compact': self.compact, 'numwant': self.compact}
      url = "%s?%s" % (url, urllib.urlencode(payload))
      res = urllib2.urlopen(url, timeout = CONNECTION_TIME_OUT).read()
      print "tracker response = ", res
      if "failure reason" not in res:
        if type(res) == str:
          res_len = len(res)
          i = 0
          while i < res_len:
            ip_port = (socket.inet_ntoa(res[i: i + 4]), byte_helper.bytes22int(res[i + 4: i + 6]))
            self.peers.add(ip_port)
            i += 6
        elif type(res) == list:
          for p in peers:
            self.peers.add((p["ip"], p["port"]))
    except Exception, e:
      print e

# tracker_list loop
# a threading run
def loop_tracker_list():
  while True:
    time.sleep(10)
    # print len(tracker_list)
    for tracker in tracker_list:
      # print "loop"
      tracker.loop()

threading.Thread(target = loop_tracker_list).start()


