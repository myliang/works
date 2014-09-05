#!/bin/python
# coding = utf-8

import os

BITS_BYTE = 8
BIT_MASK = 1

def byte_bit_index(index):
  return [index / BITS_BYTE, index % BITS_BYTE]

class Bitmap:

  def __init__(self, field_len, bitfile):
    self.field_len = field_len
    self.field = bytearray(self.field_len)
    self.used_len = 0
    self.f = bitfile
    if bitfile and os.path.exists(bitfile):
      self.__open_file('rb', self.__read_file)

  def store(self):
    if self.f and os.path.exists(self.f):
      self.__open_file('wb', self.__write_file)

  def set(self, index):
    self.__setv(index, 1)

  def get(self, index):
    byte_index, bit_index = byte_bit_index(index)

    bytev = self.field[byte_index]
    bytev = bytev >> (BITS_BYTE - bit_index)
    return bytev / BIT_MASK

  def clear(self, index):
    self.__setv(index, 0)

  def __setv(self, index, v):
    byte_index, bit_index = byte_bit_index(index)
    mask = BIT_MASK << (BITS_BYTE - bit_index)
    if v == 1:
      self.field[byte_index] = self.field[byte_index] | mask
    else: # 0
      self.field[byte_index] = self.field[byte_index] & ~mask

  def __read_file(self, f):
    read_bytes = f.read()
    while i < self.field_len:
      self.field[i] = read_bytes[i]
      i += 1

  def __write_file(self, f):
    f.write(self.field)

  def __open_file(self, mode, callback):
    try:
      f = open(self.f, mode)
      callback(f)
    finally:
      f.close()

  def __str__(self):
    flen = self.field_len - 1
    buf = []
    while flen >= 0:
      bytev = self.field[flen]
      buf.append('{0:08b}'.format(bytev))
      flen -= 1

    ','.join(buf)

# test
if __name__ == '__main__':
  print Bitmap(2, None)
