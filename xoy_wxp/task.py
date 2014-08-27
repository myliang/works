#!/bin/python
# coding = utf-8

import threading
import time

TASK_STATE = ["stop", "start", "wait", "finished"]
TASK_MAX_LIMIT = 10

# all task list
all_task_list = []

def nextWaitTaskToStart():
  i = 0
  for task in all_task_list:
    if task.isWait():
      task.start()
    elif task.isFinished():
      del all_task_list[i]
    i =+ 1

def convertStateToStartFromWait():
  time.sleep(10)
  print "convert"
  nextWaitTaskToStart()

def isOverMaxLimit():
  return len(all_task_list) > TASK_MAX_LIMIT

class Task(threading.Thread):
  def __init__(self, name, downloads):
    threading.Thread.__init__(self)
    # self.index = len(all_task_list)
    self.name = name
    self.downloads = downloads
    self.speed = 0.0
    self.state = 0 # 0 stop, 1 start, 2 wait, 3 finished

    all_task_list.append(self)

  def run(self):
    while self.state == 1:
      print threading.currentThread().getName()

  def stop(self):
    if self.isStart() or self.isWait():
      self.state = 0

  def start(self):
    if isOverMaxLimit():
      self.wait()
    elif self.isStop():
      self.state = 1
      super.start()

  def wait(self):
    if isStop():
      self.state = 2

  def finished(self):
    self.state = 3
    nextWaitTaskToStart()

  def isStop(self):
    return self.state == 0
  def isStart(self):
    return self.state == 1
  def isWait(self):
    return self.state == 2
  def isFinished(self):
    return self.state == 3

  def toAry(self):
    return [TASK_STATE[self.state], self.name, self.downloads, self.speed]


# start a thread loop all_task_list
# convert state to start from wait
threading.Thread(target = convertStateToStartFromWait).start()


# end
