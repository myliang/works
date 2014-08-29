#!/bin/python
# coding = utf-8

import threading
import time

TASK_STATE = ["stop", "start", "wait", "finished", "delete"]
TASK_MAX_LIMIT = 10

# all task list
all_task_list = []

def allUnFinishedTasks():
  return [t for t in all_task_list if t.isUnFinished()]

# loop element
# if node is wait then start
# else if node is finished remove it from all_task_list
def nextWaitTaskToStart():
  if isOverMaxLimit():
    return

  i = 0
  for task in all_task_list:
    if task.isWait():
      task.start()
    elif task.isFinished():
      all_task_list[i].finished()
    i += 1

# convert state to start from wait
def convertStateToStartFromWait():
  time.sleep(10)
  nextWaitTaskToStart()

# is over task max limit
def isOverMaxLimit():
  return len(all_task_list) > TASK_MAX_LIMIT

# start task list by task indexes
# Simple: ary => [1, 3, 5]
def starts(ary):
  for index in ary:
    all_task_list[index].start()
# stop task list by task indexes
# Simple: ary => [1, 3, 5]
def stops(ary):
  for index in ary:
    all_task_list[index].stop()
# delete task list by task indexes
# Simple: ary => [1, 3, 5]
def deletes(ary):
  for index in ary:
    all_task_list[index].delete()


# task thread class
class Task(threading.Thread):
  def __init__(self, name, downloads):
    threading.Thread.__init__(self)
    # self.index = len(all_task_list)
    self.name = name
    self.downloads = downloads
    self.speed = 0.0
    self.state = 0 # 0 stop, 1 start, 2 wait, 3 finished, 4 delete

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
    elif self.isStop() or self.isWait():
      self.state = 1
      # super(Task, self).start()

  def wait(self):
    if self.isStop():
      self.state = 2

  def delete(self):
    self.state = 4
    nextWaitTaskToStart()

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
  def isUnFinished(self):
    return self.isStart() or self.isStop() or self.isWait()

  def toAry(self):
    return [TASK_STATE[self.state], self.name, self.downloads, self.speed]


# start a thread loop all_task_list
# convert state to start from wait
threading.Thread(target = convertStateToStartFromWait).start()

print "task end"

# end
