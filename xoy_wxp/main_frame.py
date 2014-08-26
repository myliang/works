#!/bin/python
# coding = utf-8

import wx
import os
import sys

import new_frame
import content_list

# Action Bar Icons/holo_light/01_core_new/drawable-mdpi
# Action Bar Icons/holo_light/09_media_pause/drawable-mdpi
# Action Bar Icons/holo_light/09_media_play/drawable-mdpi

def getImagePath(filename):
  return os.path.join(os.getcwd(), "icons/holo_light", filename)

def getBitmapImage(filename):
  print getImagePath(filename)
  return wx.Bitmap(getImagePath(filename))

class MainFrame(wx.Frame):
  def __init__(self, parent, title):
    wx.Frame.__init__(self, parent, title = title, size = (500, 600))
    toolBar = self.CreateToolBar()

    # new, start, stop, delete, setting
    toolNew = toolBar.AddLabelTool(wx.ID_ANY, 'new', getBitmapImage("09_media_add_to_queue/drawable-mdpi/ic_action_add_to_queue.png"))
    toolStart = toolBar.AddLabelTool(wx.ID_ANY, 'start', getBitmapImage("09_media_play/drawable-mdpi/ic_action_play.png"))
    toolStop = toolBar.AddLabelTool(wx.ID_ANY, 'stop', getBitmapImage("09_media_pause/drawable-mdpi/ic_action_pause.png"))
    toolBar.Realize()

    # bind event
    self.Bind(wx.EVT_TOOL, self.onNew, toolNew)
    self.Bind(wx.EVT_TOOL, self.onStart, toolStart)
    self.Bind(wx.EVT_TOOL, self.onStop, toolStop)

    # self.SetMenuBar(menuBar)
    panel = wx.Panel(self)
    content_list.ContentList(panel)
    self.Show(True)

  def onNew(self, event):
    new_frame.NewFrame(self, "new")

  def onStart(self, event):
    print "start"

  def onStop(self, event):
    print "stop"

if __name__ == '__main__':
  app = wx.App(False)
  frame = MainFrame(None, "xoy")
  app.MainLoop()
