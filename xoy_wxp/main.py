__author__ = 'myliang'

import wx
import xoy.ui

if __name__ == '__main__':
  app = wx.App(False)
  frame = xoy.ui.MainFrame(None, "xoy")
  app.MainLoop()
