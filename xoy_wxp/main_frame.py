#!/bin/python
# coding = utf-8

import wx
import os

def getImagePath(filename):
  os.path.join(os.getcwd(), "icons", filename)

class MainFrame(wx.Frame):
  def __init__(self, parent, title):
    wx.Frame.__init__(self, parent, title = title, size = (500, 600))
    toolBar = self.CreateToolBar()
    # new, start, stop, delete, setting
    qtool = toolBar.AddLabelTool(wx.ID_EXIT, '', wx.Bitmap("/home/myliang/workspaces/wxpython/test/exit.png"))
    toolBar.Realize()

    # self.SetMenuBar(menuBar)
    self.Show(True)

if __name__ == '__main__':
  app = wx.App(False)
  frame = MainFrame(None, "xoy")
  app.MainLoop()
