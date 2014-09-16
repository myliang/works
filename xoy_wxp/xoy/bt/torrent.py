#!/bin/python
# -*- coding:utf8 -*-

import sys
import hashlib

from bitmap import Bitmap
from bencode import BEncode

reload(sys)
sys.setdefaultencoding('utf8')

# convert data to becode from file
def to_bencode_from_file(filename):
  with open(filename, 'rb') as f:
    return BEncode(f.read())
  return None
  # f = None
  # try:
  #   f = open(filename, 'rb')
  #   read_bytes = f.read()
  #   return BEncode(read_bytes)
  # finally:
  #   if f:
  #     f.close()
  # return None

# class torrent
class Torrent:
  def __init__(self, filename):
    bencode = to_bencode_from_file(filename)
    d = bencode.parse()
    self.encoding = d.get('encoding', 'utf-8')
    self.comment = d['comment'].decode(self.encoding).encode('utf-8')
    self.creation_date = d['creation date']
    self.created_by = d['created by']

    self.announces = []
    if d['announce-list']:
      for v in d['announce-list']:
        self.announces.append(v[0])
    else:
      self.announces.append(d['announce'])

    # info hash
    self.info_hash = hashlib.sha1(bencode.info).digest()
    self.name = d['info']['name']
    if self.name:
      self.name = self.name.decode(self.encoding).encode('utf-8')
    self.piece_length = d['info']['piece length']
    self.pieces = d['info']['pieces']
    self.files = []

    for f in d['info']['files']:
      path = '/'.join(f['path']).decode(self.encoding).encode('utf-8')
      self.files.append(TorrentFile(path, f['length']))

    # bitfield init
    self.bitfield = Bitmap(self.blocks_len())

    # local peer
    # remote peers
    # active peers
    self.peers = []

  def __repr__(self):
    return ''.join(['%s:%s\n' % item for item in self.__dict__.items() if item[0] != "pieces" and item[0] != 'bitfield'])

  def blocks_len(self):
    pl = len(self.pieces)
    length = pl/20
    if pl % 20:
      length += 1
    return length

  def down_rate(self):
    return reduce(lambda x, y: x.down_rate() + y.down_rate(), self.peers)
  def up_rate(self):
    return reduce(lambda x, y: x.up_rate() + y.up_rate(), self.peers)


class TorrentFile:

  def __init__(self, path, length):
    self.path = path
    self.length = length

  #def __str__(self):
  #  return ''.join(["%s:%s\n" % item for item in self.__dict__.items()])
  def __repr__(self):
    return ''.join(["%s:%s\n" % item for item in self.__dict__.items()])

if __name__ == '__main__':
  torrent = Torrent(sys.argv[1])
  print torrent
  print torrent.blocks_len()
