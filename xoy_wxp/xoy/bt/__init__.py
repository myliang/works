__author__ = 'myliang'

from bitmap import Bitmap
from torrent import Torrent
from tracker import Tracker

import pickle
import os
import sys

# path = os.path.split(os.path.realpath(__file__))[0].split('/')
# path = path[0: len(path) - 1]
# path = '/'.join(path)
# sys.path.append(path)

from xoy import config

# torrent store and recover
def torrent_pickle_path(dest, torrent):
  return os.path.join(dest, torrent.name, ".pickle")

def torrent_store(torrent, dest):
  with open(torrent_pickle_path(dest, torrent) , 'wb') as f:
    pickle.dump(torrent, f)

def torrent_recover(torrent, dest):
  with open(torrent_pickle_path(dest, torrent) , 'wb') as f:
    return pickle.load(f)

if __name__ == '__main__':
  torrent = Torrent(sys.argv[1])
  Tracker(torrent, config.bt.peer_id, config.bt.port, 0, 0, 10000)
