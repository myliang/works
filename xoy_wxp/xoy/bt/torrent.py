#!/bin/python
# coding = utf-8

import sys
import hashlib

reload(sys)
sys.setdefaultencoding('utf8')

def to_torrent_from_file(filename):
  return Torrent(to_bencode_from_file(filename))

# convert data to becode from file
def to_bencode_from_file(filename):
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
    self.comment = d['comment']
    self.creation_date = d['creation date']
    self.created_by = d['created by']
    self.encoding = d.get('encoding')

    self.announces = []
    if d['announce-list']:
      for v in d['announce-list']:
        self.announces.append(v[0])
    else:
      self.announces.append(d['announce'])

    # info hash
    self.info_hash = hashlib.sha1(bencode.info).hexdigest()
    self.name = d['info']['name']
    self.piece_length = d['info']['piece length']
    self.pieces = d['info']['pieces']
    self.files = []

    for f in d['info']['files']:
      self.files.append(TorrentFile('/'.join(f['path']), f['length']))


  def __repr__(self):
    return ''.join(['%s:%s\n' % item for item in self.__dict__.items() if item[0] != "pieces" ])

  def pieces_len(self):
    return len(self.pieces)/20 + 1


class TorrentFile:

  def __init__(self, path, length):
    self.path = path
    self.length = length

  #def __str__(self):
  #  return ''.join(["%s:%s\n" % item for item in self.__dict__.items()])
  def __repr__(self):
    return ''.join(["%s:%s\n" % item for item in self.__dict__.items()])

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


if __name__ == '__main__':
  torrent = to_torrent_from_file(sys.argv[1])
  print torrent
  print torrent.pieces_len()
