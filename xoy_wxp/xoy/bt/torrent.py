#!/bin/python
# coding = utf-8

import sys
import hashlib

def toTorrentFromFile(filename):
  return Torrent(toBEncodeFromFile(filename))

# convert data to becode from file
def toBEncodeFromFile(filename):
  f = None
  try:
    f = open(filename, 'rb')
    read_bytes = f.read()
    return BEncode(read_bytes)
  finally:
    if f:
      f.close()
  return None

# class torrent
class Torrent:
  def __init__(self, bencode):
    d = bencode.parse()
    self.comment = d['comment'].decode('utf-8')
    self.creation_date = d['creation date']
    self.created_by = d['created by'].decode('utf-8')
    self.encoding = d['encoding'].decode('utf-8')

    self.announces = []
    if d['announce-list']:
      for v in d['announce-list']:
        self.announces.append(v[0])
    else:
      self.announces.append(d['announce'].decode('utf-8'))

    # info hash
    self.info_hash = hashlib.sha1(bencode.info).hexdigest()
    self.name = d['info']['name']
    self.pieces = d['info']['pieces']
    self.piece_length = d['info']['piece length']
    self.files = d['info']['files']
    print self.files


# class bencode
class BEncode:

  def __init__(self, value):
    self.value = value
    self.index = 0

  def parse(self):
    v = self.indexValue()
    if v == 'd':
      return self.parseDict()
    elif v == 'l':
      return self.parseList()
    elif v == 'i':
      return self.parseInt()
    else:
      return self.parseString()


  def parseDict(self):
    kv = {}
    self.indexInc()
    while 'e' != self.indexValue():
      key = self.parseString()
      begin = self.index
      value = self.parse()
      if 'info' == key:
        self.info = self.value[begin:self.index]

      kv[key] = value
    self.indexInc()
    return kv

  def parseList(self):
    v = []
    self.indexInc()
    while 'e' != self.indexValue():
      v.append(self.parse())
    self.indexInc()
    return v

  def parseString(self):
    length = self.readInt()
    self.indexInc()
    begin = self.index
    self.index += length
    return self.value[begin:self.index]

  def parseInt(self):
    self.indexInc()
    v = self.readInt()
    self.indexInc()
    return v

  def indexValue(self):
    return self.value[self.index]

  def indexInc(self):
    self.index += 1

  def readInt(self):
    v = 0
    while self.indexValue().isdigit():
      v = v * 10 + int(self.indexValue())
      self.indexInc()
    return v


if __name__ == '__main__':
  toTorrentFromFile(sys.argv[1])
