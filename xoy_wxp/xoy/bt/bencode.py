#!/bin/python
# -*- coding:utf8 -*-

import sys
import hashlib
# class bencode
class BEncode:

  def __init__(self, value):
    self.value = value
    self.index = 0

  def parse(self):
    v = self.index_value()
    if v == 'd':
      return self.parse_dict()
    elif v == 'l':
      return self.parse_list()
    elif v == 'i':
      return self.parse_int()
    else:
      return self.parse_string()


  def parse_dict(self):
    kv = {}
    self.index_inc()
    while 'e' != self.index_value():
      key = self.parse_string()
      begin = self.index
      value = self.parse()
      if 'info' == key:
        self.info = self.value[begin:self.index]

      kv[key] = value
    self.index_inc()
    return kv

  def parse_list(self):
    v = []
    self.index_inc()
    while 'e' != self.index_value():
      v.append(self.parse())
    self.index_inc()
    return v

  def parse_string(self):
    length = self.read_int()
    self.index_inc()
    begin = self.index
    self.index += length
    return self.value[begin:self.index]

  def parse_int(self):
    self.index_inc()
    v = self.read_int()
    self.index_inc()
    return v

  def index_value(self):
    return self.value[self.index]

  def index_inc(self):
    self.index += 1

  def read_int(self):
    v = 0
    while self.index_value().isdigit():
      v = v * 10 + int(self.index_value())
      self.index_inc()
    return v



