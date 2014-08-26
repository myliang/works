#!/bin/python
# coding = utf-8

import wx
import os
import sys

class NewFrame(wx.Frame):
  def __init__(self, parent, title):
    wx.Frame.__init__(self, parent, title = title, size = (500, 200))
    panel = wx.Panel(self)
    self.address = wx.TextCtrl(panel, -1, "url address", pos = (20, 10), size = (460, 120), style = wx.TE_MULTILINE)
    btnOk = wx.Button(panel, label = "ok", pos = (20, 150))
    btnOpen = wx.Button(panel, label = "open file(bt)", pos = (120, 150))

    panel.Bind(wx.EVT_BUTTON, self.onBtnOk, btnOk)
    panel.Bind(wx.EVT_BUTTON, self.onBtnOpen, btnOpen)

    self.Show(True)

  def onBtnOk(self, event):
    print self.address.GetValue()
    self.Close()

  def onBtnOpen(self, event):
    print "onBtnOpen"
    dlg = wx.FileDialog(self, "choose files", "", "", "*.*", wx.OPEN | wx.FD_MULTIPLE)
    if dlg.ShowModal() == wx.ID_OK:
      print dlg.GetPaths()
    dlg.Destroy()
    self.Close()


