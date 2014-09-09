__author__ = 'myliang'

from bitmap import Bitmap
from torrent import Torrent
from tracker import Tracker

def bt_start(filename, dest_dir):
  torrent = Torrent(filename)
  Bitmap(torrent.blocks())
