#!/bin/python
# coding = utf-8

import wx
import os
import sys

class NewFrame(wx.Dialog):
  def __init__(self, parent, title):
    wx.Dialog.__init__(self, parent, title = title, size = (500, 200),
        style = wx.DEFAULT_DIALOG_STYLE)
    self.address = wx.TextCtrl(self, -1, "url address", pos = (20, 10), size = (460, 120), style = wx.TE_MULTILINE)
    btnOk = wx.Button(self, label = "ok", pos = (20, 150))
    btnOpen = wx.Button(self, label = "open file(bt)", pos = (120, 150))

    self.Bind(wx.EVT_BUTTON, self.onBtnOk, btnOk)
    self.Bind(wx.EVT_BUTTON, self.onBtnOpen, btnOpen)

    self.ShowModal()

  def onBtnOk(self, event):
    self.values = [self.address.GetValue()]
    self.Close()

  def onBtnOpen(self, event):
    dlg = wx.FileDialog(self, "choose files", "", "", "*.*", wx.OPEN | wx.FD_MULTIPLE)
    if dlg.ShowModal() == wx.ID_OK:
      self.values = dlg.GetPaths()
    dlg.Destroy()
    self.Close()


