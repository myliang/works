#!/bin/python
# coding = utf-8

import os
import ConfigParser

def read_config():
  config = ConfigParser.ConfigParser()
  path = os.path.split(os.path.realpath(__file__))[0].split('/')
  path = os.path.join('/'.join(path[0: len(path) - 1]), "config.conf")
  # print path
  config.read(path)
  return config

# global config
config = read_config()

# bt
BT_CONFIG_FIELDS = ['peer_id', 'port']

class ConfigBt:
  def __init__(self):
    for field in BT_CONFIG_FIELDS:
      setattr(self, field, config.get('bt', field))


bt = ConfigBt()

if __name__ == '__main__':
  print bt.peer_id
  print bt.port
