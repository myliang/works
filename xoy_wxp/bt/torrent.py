#!/bin/python
# coding = utf-8

class BEncode:

  def __init__(self, value):
    self.value = value
    self.index = 0

  def parse(self):
    v = self.indexValue()
    if v == 'd':
      return parseDict()
    elif v == 'l':
      return parseList()
    elif v == 'i':
      return parseInt()
    else:
      return parseString()


  def parseDict(self):
    kv = {}
    self.indexInc()
    while 'e' != self.indexValue():
      key = parseString()
      value = parse()
      kv[key] = value
    self.indexInc()
    return kv

  def parseList(self):
    v = []
    self.indexInc()
    while 'e' != sef.indexValue():
      v.append(parse())
    self.indexInc()
    return v

  def parseString(self):
    length = readInt()
    self.indexInc()
    begin = self.index
    self.index += length
    return self.value[begin, self.index]

  def parseInt(self):
    self.indexInc()
    return readInt()

  def indexValue(self):
    return self.value[self.index]

  def indexInc(self):
    self.index += 1

  def readInt(self):
    v = 0
    while self.indexValue().isdigit():
      v = v * 10 + (self.indexValue() - '0')
      self.index++
    return v
