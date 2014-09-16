#!/bin/python
# -*- coding:utf8 -*-

import time
import random
import socket
import select

from xoy import byte_helper, config

CONNECTION_TIME_OUT = int(config.bt.connection_time_out)

# udp request
# req1 : Connecttion_ID(8字节）   0（4字节） Transaction_ID（4字节）
# res1 : 0（4字节） Transaction_ID（4字节） Connecttion_ID(8字节）
# req2 : InfoHash   20字节, PeerID     20字节, DownLoad  8字节, Left   8字节, UpLoad  8字节, Event    4字节, IP       4字节,
#       下面是8个字节的空(88-96字节是空的内容), port      4字节, 然后是可选内容 主要是服务器的HostName, UserName, PassWord
# res2 : 1（4字节） Transaction_ID（4字节） Interval Time(4) 当前下载数目(4) 种子数(4) 然后是IP（4字节）和Port(2字节）)

class UdpTracker:
  def __init__(self, ip_port):
    self.sockfd = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    print ip_port
    try:
      self.sockfd.connect(ip_port)
    except Exception, e:
      print e


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


# static method

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

