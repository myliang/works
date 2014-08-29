#!/bin/python
# coding = utf-8

import wx
import wx.grid

# GRID_COLUMNS=["fileName", "processing", "spead"]
import task


class ContentList(wx.ListCtrl):
  def __init__(self, parent):
    wx.ListCtrl.__init__(self, parent, -1, style = wx.LC_REPORT | wx.LC_NO_HEADER, size = (500, 500))
    self.InsertColumn(0, "state")
    self.InsertColumn(1, "filename")
    self.InsertColumn(2, "processing")
    self.InsertColumn(3, "spead")

    self.SetColumnWidth(0, 40)
    self.SetColumnWidth(1, 300)
    self.SetColumnWidth(2, 80)
    self.SetColumnWidth(3, 80)

    # bind event
    self.Bind(wx.EVT_LIST_ITEM_RIGHT_CLICK, self.onPopupMenu)

  # overrite method
  def Append(self, task):
    wx.ListCtrl.Append(self, task.toAry())


  # customer methods
  def onPopupMenu(self, event):
    menu = wx.Menu()
    menuStart = menu.Append(wx.ID_ANY, "start")
    menuStop = menu.Append(wx.ID_ANY, "stop")
    menuDelete = menu.Append(wx.ID_ANY, "delete")

    self.Bind(wx.EVT_MENU, self.onStart, menuStart)
    self.Bind(wx.EVT_MENU, self.onStop, menuStop)
    self.Bind(wx.EVT_MENU, self.onDelete, menuDelete)

    self.PopupMenu(menu)
    menu.Destroy()


  def getSelectedValues(self):
    # selected
    item = -1;
    values = []
    while True:
      item = self.GetNextSelected(item)
      if item == -1:
        break;
      print "selected.item = ", item
      values.append(item)
    return values

  def onStart(self, event):
    task.starts(self.getSelectedValues())
    self.refreshAllData()

  def onStop(self, event):
    task.stops(self.getSelectedValues())
    self.refreshAllData()

  def onDelete(self, event):
    task.deletes(self.getSelectedValues())
    self.refreshAllData()

  def refreshAllData(self):
    self.DeleteAllItems()
    for t in task.allUnFinishedTasks():
      self.Append(t)


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

