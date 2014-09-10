#!/bin/python
# coding = utf-8

# bytes 2 to int
def bytes22int(bytes2):
  return (bytes2[0] << 8) & 0xffff + bytes2[1] & 0xff

# to bytes 4 from int
def int2bytes4(iv):
  return [(iv >> 24) & 0xff, (iv >> 16) & 0xff, (iv >> 8) & 0xff, iv & 0xff]

