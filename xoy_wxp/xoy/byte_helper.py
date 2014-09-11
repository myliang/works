#!/bin/python
# coding = utf-8

# bytes 2 to int
def bytes22int(bs):
  return (bs[0] << 8) & 0xffff + bs[1] & 0xff

# bytes 4 to int
def bytes42int(bs):
  return (bs[0] << 24) & 0xffffffff + (bs[1] << 16) & 0xffffff + (bs[2] << 8) & 0xffff + bs[3] & 0xff

# to bytes 4 from int
def int2bytes2(iv):
  return [(iv >> 8) & 0xff, iv & 0xff]

# to bytes 4 from int
def int2bytes4(iv):
  return [(iv >> 24) & 0xff, (iv >> 16) & 0xff, (iv >> 8) & 0xff, iv & 0xff]

# to bytes 4 from int
def int2bytes8(iv):
  return [(iv >> 56) & 0xff, (iv >> 48) & 0xff, (iv >> 40) & 0xff, (iv >> 32) & 0xff,
      (iv >> 24) & 0xff, (iv >> 16) & 0xff, (iv >> 8) & 0xff, iv & 0xff]

