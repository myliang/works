#!/bin/python
# coding = utf-8

import socket

# creat tcp connect
def tcp_connect(ip, port):
  sockfd = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
  sockfd.connect(ip, port)
  return sockfd

# to bytes 4 from int
def int2bytes4(iv):
  return [(iv >> 24) & 0xff, (iv >> 16) & 0xff, (iv >> 8) & 0xff, iv & 0xff]

def hand_shake(info_hash, peer_id ):
  byteary = bytearray(68)
  byteary[0] = 19
  byteary[1:20] = "BitTorrent protocol"
  byteary[28:48] = info_hash
  byteary[48:68] = peer_id
  return byteary

def keep_alive():
  return bytearray(4)

def choke():
  return __byte_array(0x00)

def unchoke():
  return __byte_array(0x01)

def interested():
  return __byte_array(0x02)

def uninterested():
  return __byte_array(0x03)

def have(index):
  byteary = bytearray(9)
  byteary[3] = 0x05
  byteary[4] = 0x04
  byteary[5:] = int2bytes4(index)
  return byteary

def bitfield(bitmap, peer):
  byteary = bytearray(5 + bitmap.field_len)
  byteary[0:4] = int2bytes4(bitmap.field_len + 1)
  byteary[4] = 0x05
  byteary[5:] = bitmap.field

def piece(blocks, block_len, index, begin):
  byteary = bytearray(1024)
  byteary[0:4] = int2bytes4(block_len + 9)
  byteary[4] = 0x07
  byteary[5:9] = int2bytes4(index)
  byteary[9:13] = int2bytes4(begin)
  tmp = 13
  for block in blocks:
    byteary[tmp:] = block
    tmp += len(block)

def request(index, begin, length):
  return __request_cancel(0x06, index, begin, length)

def cancel(index, begin, length):
  return __request_cancel(0x08, index, begin, length)

# private method
def __request_cancel(fv, index, begin, length):
  byteary = bytearray(17)
  byteary[3] = 0x0d
  byteary[4] = fv
  byteary[5:9] = int2bytes4(index)
  byteary[9:13] = int2bytes4(begin)
  byteary[13:17] = int2bytes4(length)
  return byteary

def __byte_array(fv):
  byteary = bytearray(5)
  byteary[3] = 0x01
  byteary[4] = fv
  return byteary




