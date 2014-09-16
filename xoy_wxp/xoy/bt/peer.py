#!/bin/python
# -*- coding:utf8 -*-

from xoy import config

PEER_INITIAL = -1   # 表明处于初始化状态
PEER_HALFSHAKED = 0   # 表明处于半握手状态
PEER_HANDSHAKEE = 1   # 表明处于全握手状态
PEER_SENDBITFIELD = 2   # 表明处于已发送位图状态
PEER_RECVBITFIELD = 3   # 表明处于已接收位图状态
PEER_DATA = 4   # 表明处于与peer交换数据的状态
PEER_CLOSING = 5 # 表明处于即将与peer断开的状态

class Peer:
  def __init__(self, ip, port, sockfd, peer_id):
    self.ip = ip
    self.port = port
    self.sockfd = sockfd
    self.id = peer_id

    # state
    self.am_choking = 1   # 是否将peer阻塞
    self.am_interested = 0  # 是否对peer感兴趣
    self.peer_choking = 1   # 是否被peer阻塞
    self.peer_interested = 0 # 是否被peer感兴趣

    # bitmap
    self.bitmap = None
    self.state = PEER_INITIAL

    # requestPieces
    self.request_pieces = [] # 向peer请求数据的队列
    self.requested_pieces = [] # 被peer请求数据的队列

    self.down_total = 0 # 从该peer下载的数据的总和
    slef.up_total = 0 # 向该peer上传的数据的总和

    # time
    self.last_recv_time = None # 最近一次接收到peer消息的时间
    self.last_send_time = None # 最近一次发送消息给peer的时间

    self.last_down_time = None #  最近下载数据的开始时间
    self.last_up_time = None # 最近上传数据的开始时间

    self.down_count = 0 # 本计时周期从peer下载的数据的字节数
    self.up_count = 0 # 本计时周期向peer上传的数据的字节数

  def is_not_request(self):
    return self.am_interested == 0 or self.peer_choking == 1

  def down_rate(self):
    self.down_count/config.bt.rate_cycle_time
  def up_rate(self):
    self.up_count/config.bt.rate_cycle_time


class RequestPiece:
  def __init__(self, index, begin, length):
    self.index = index # 请求的piece的索引
    self.begin = begin # 请求的piece的偏移
    self.length = length # 请求的长度,一般为16KB

