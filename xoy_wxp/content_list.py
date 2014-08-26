#!/bin/python
# coding = utf-8

import wx
import wx.grid

# GRID_COLUMNS=["fileName", "processing", "spead"]

class ContentList(wx.ListCtrl):
  def __init__(self, parent):
    wx.ListCtrl.__init__(self, parent, -1, style = wx.LC_REPORT | wx.LC_NO_HEADER, size = (500, 500))
    self.InsertColumn(0, "index")
    self.InsertColumn(1, "filename")
    self.InsertColumn(2, "processing")
    self.InsertColumn(3, "spead")

    self.SetColumnWidth(0, 20)
    self.SetColumnWidth(1, 320)
    self.SetColumnWidth(2, 80)
    self.SetColumnWidth(3, 80)

    self.Append([0, "Dirt", 80, 100])
    self.Append([1, "Dirt", 80, 100])
    self.Append([2, "Dirt", 80, 100])
    self.Append([3, "Dirt", 80, 100])
    self.Append([4, "Dirt", 80, 100])

    # bind event
    self.Bind(wx.EVT_LIST_ITEM_RIGHT_CLICK, self.onPopupMenu)

  def onPopupMenu(self, event):
    menu = wx.Menu()
    menuStart = menu.Append(wx.ID_ANY, "start")
    menuStop = menu.Append(wx.ID_ANY, "stop")
    menuDelete = menu.Append(wx.ID_ANY, "delete")

    self.PopupMenu(menu)
    menu.Destroy()

    # selected
    item = -1;
    while True:
      item = self.GetNextSelected(item)
      if item == -1:
        break;
      print "selected.item = ", item



class ContentGrid(wx.grid.Grid):
  def __init__(self, parent):
    wx.grid.Grid.__init__(self, parent, -1)
    self.CreateGrid(2, 3)

    # hide label
    self.SetRowLabelSize(0)
    self.SetColLabelSize(0)

    self.EnableEditing(False)
    # self.EnableGridLines(False)

    self.Bind(wx.grid.EVT_GRID_CELL_RIGHT_CLICK, self.onPopupMenu)

  def onPopupMenu(self, event):
    menu = wx.Menu()
    menuStart = menu.Append(wx.ID_ANY, "start")
    menuStop = menu.Append(wx.ID_ANY, "stop")
    menuDelete = menu.Append(wx.ID_ANY, "delete")

    self.PopupMenu(menu)
    menu.Destroy()

