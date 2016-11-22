// Copyright (C) 2012-2014 F32 (feng32tc@gmail.com)
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License version 3 as
// published by the Free Software Foundation;
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA 

#include "stdafx.h"
#include "ProcessView.h"
#include "Utils.h"

HWND ProcessView::_hList;
bool ProcessView::_hideProcess;
bool ProcessView::_prevHideProcess;

// Initialize process list & ListView control
void ProcessView::Init(HWND hList)
{
    // Init ListView Headers
    _hList = hList;
    _hideProcess = false;
    _prevHideProcess = false;

    Utils::ListViewInit(_hList, FALSE, 5, 
        _T("UID"), _T("Process"), _T("Tx Rate"), _T("Rx Rate"), _T("Full Path"), 
        50, 150, 70, 70, 400);

    // Init Model
    ProcessModel::Init();
}

// ListView operations - insert an item & update an item
void ProcessView::ListViewInsert(const ProcessModel::ProcessItem &item)
{
    // Prepare Columns
    TCHAR szColumn[5][MAX_PATH]={ 0 };

    _stprintf_s(szColumn[0], MAX_PATH, _T("%d"), item.puid);
    _stprintf_s(szColumn[1], MAX_PATH, _T("%s"), item.name);

    if( item.active )
    {
        _stprintf_s(szColumn[2], MAX_PATH, _T("%d.%d"), 
            item.prevTxRate / 1024, (item.prevTxRate % 1024 + 51) / 108);
        _stprintf_s(szColumn[3], MAX_PATH, _T("%d.%d"), 
            item.prevRxRate / 1024, (item.prevRxRate % 1024 + 51) / 108);
        _tcscpy_s(szColumn[4], MAX_PATH, item.fullPath);
    }
    else
    {
        _tcscpy_s(szColumn[2], MAX_PATH, _T("-"));
        _tcscpy_s(szColumn[3], MAX_PATH, _T("-"));
        _tcscpy_s(szColumn[4], MAX_PATH, item.fullPath);
    }

    Utils::ListViewAppend(_hList, 5, 
        szColumn[0], szColumn[1], szColumn[2], szColumn[3], szColumn[4]);
}

void ProcessView::ListViewUpdate(int index, const ProcessModel::ProcessItem &item)
{
    // Prepare Columns
    TCHAR szColumn[5][MAX_PATH];

    if( item.active )
    {
        _stprintf_s(szColumn[2], MAX_PATH, _T("%d.%d"), 
            item.prevTxRate / 1024, (item.prevTxRate % 1024 + 51) / 108);
        _stprintf_s(szColumn[3], MAX_PATH, _T("%d.%d"), 
            item.prevRxRate / 1024, (item.prevRxRate % 1024 + 51) / 108);
        _tcscpy_s(szColumn[4], MAX_PATH, item.fullPath);
    }
    else
    {
        _tcscpy_s(szColumn[2], MAX_PATH, _T("-"));
        _tcscpy_s(szColumn[3], MAX_PATH, _T("-"));
        _tcscpy_s(szColumn[4], MAX_PATH, item.fullPath);
    }

    Utils::ListViewUpdate(_hList, index, 5, 
        FALSE, FALSE, TRUE, TRUE, TRUE,
        szColumn[0], szColumn[1], szColumn[2], szColumn[3], szColumn[4]);
}

void ProcessView::Update(bool init, bool redraw)
{
    // Update is called in:
    //   1. ProcessModel::Init ProfileInit
    //   2. ProcessModel::OnTimer
    //   3. ProcessModel::ShowProcess(puid)
    //   4. ProcessModel::HideProcess(puid)
    //   5. ProcessView::ShowProcesses
    //   6. ProcessView::HideProcesses
    std::vector<ProcessModel::ProcessItem> processes;
    ProcessModel::Export(processes);

    if (init) // 1. Init
    {
        Utils::ListViewClear(_hList);
        for (unsigned int i = 0; i < processes.size(); i++)
        {
            if (!_hideProcess || !processes[i].hidden)
            {
                ListViewInsert(processes[i]);
            }
        }
    }
    else if (redraw == false && _hideProcess != _prevHideProcess) // 5. ShowProcesses & 
    {                                                             // 6. HideProcesses
        if (_hideProcess) // Delete hidden processes
        {
            Utils::ListViewClear(_hList);
            for (unsigned int i = 0; i < processes.size(); i++)
            {
                if (!processes[i].hidden)
                {
                    ListViewInsert(processes[i]);
                }
            }
        }
        else // Show gray hidden processes
        {
            Utils::ListViewClear(_hList);
            for (unsigned int i = 0; i < processes.size(); i++)
            {
                ListViewInsert(processes[i]);
            }
        }
        _prevHideProcess = _hideProcess;
    }
    else if (redraw == false) // 2. OnTimer
    {
        if (!_hideProcess) // hidden processes are displayed as grayed items
        {
            int rows = Utils::ListViewGetRowCount(_hList);
            for (int i = 0; i < rows; i++)
            {
                if (processes[i].dirty)
                {
                    ListViewUpdate(i, processes[i]);
                }
            }

            for (unsigned int i = rows; i < processes.size(); i++)
            {
                ListViewInsert(processes[i]);
            }
        }
        else
        {
            // Update Existent Items
            int rows = Utils::ListViewGetRowCount(_hList);
            for (int i = 0; i < rows; i++)
            {
                TCHAR szPUID[16];
                Utils::ListViewGetText(_hList, i, 0, szPUID, 16);
                int puid = _tstoi(szPUID);

                for (unsigned int j = 0; j < processes.size(); j++)
                {
                    if (puid == processes[j].puid && processes[j].dirty)
                    {
                        ListViewUpdate(i, processes[j]);
                    }
                }
            }

            // Insert New Items
            for (unsigned int i = 0; i < processes.size(); i++)
            {
                if (processes[i].hidden == false)
                {
                    int rows = Utils::ListViewGetRowCount(_hList);
                    bool found = false;

                    for (int j = 0; j < rows; j++)
                    {
                        TCHAR szPUID[16];
                        Utils::ListViewGetText(_hList, j, 0, szPUID, 16);
                        int puid = _tstoi(szPUID);
                        if (puid == processes[i].puid)
                        {
                            found = true;
                            break;
                        }
                    }

                    if (!found)
                    {
                        ListViewInsert(processes[i]);
                    }
                }
            }
        }
    }
    else // 3. ShowProcess and 4. HideProcess
    {
        UpdateWindow(_hList);
    }
}

void ProcessView::HideProcesses()
{
    _prevHideProcess = _hideProcess;
    _hideProcess = true;
    Update(false, false);
}

void ProcessView::ShowProcesses()
{
    _prevHideProcess = _hideProcess;
    _hideProcess = false;
    Update(false, false);
}

bool ProcessView::IsHidden()
{
    return _hideProcess;
}