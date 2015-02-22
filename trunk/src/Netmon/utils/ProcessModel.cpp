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
#include "ProcessModel.h"
#include "ProcessView.h"
#include "ProcessCache.h"
#include "Profile.h"
#include "Utils.h"

#pragma region Members of Process

std::vector<ProcessModel::ProcessItem> ProcessModel::_processes;
CRITICAL_SECTION ProcessModel::_cs;

#pragma endregion

extern Profile g_profile;

void ProcessModel::Init()
{
    // Init Process List
    TCHAR command[256] = TEXT("Select * From Process;");

    SQLiteRow row;
    row.InsertType(SQLiteRow::TYPE_INT32);
    row.InsertType(SQLiteRow::TYPE_STRING);
    row.InsertType(SQLiteRow::TYPE_STRING);

    SQLite::Select(command, &row, InitCallback);

    // Init Critical Section
    InitializeCriticalSection(&_cs);

    // Update View
    ProcessView::Update(true);
}

void ProcessModel::InitCallback(SQLiteRow *row)
{
    // Create a ProcessItem and Fill
    ProcessItem item;

    RtlZeroMemory(&item, sizeof(item));

    // - active & uid
    item.active = false;
    item.puid = row->GetDataInt32(0);

    // - name & fullPath
    _tcscpy_s(item.name, MAX_PATH, row->GetDataStr(1));
    _tcscpy_s(item.fullPath, MAX_PATH, row->GetDataStr(2));

    // Add Process
    _processes.push_back(item);
}

void ProcessModel::Lock()
{
    EnterCriticalSection(&_cs);
}

void ProcessModel::Unlock()
{
    LeaveCriticalSection(&_cs);
}

// Event handlers
void ProcessModel::OnPacket(PacketInfoEx *pi)
{
    int index = GetProcessIndex(pi->puid);

    if (index == -1) // A new process
    {
        // Insert a ProcessItem
        ProcessItem item;

        RtlZeroMemory(&item, sizeof(item));

        item.active = true;
        item.dirty = false;
        item.pid = pi->pid; // The first pid is logged
        item.puid = pi->puid;

        _tcscpy_s(item.name, MAX_PATH, pi->name);
        _tcscpy_s(item.fullPath, MAX_PATH, pi->fullPath);

        item.hidden = false;

        item.txRate = 0;
        item.rxRate = 0;
        item.prevTxRate = 0;
        item.prevRxRate = 0;

        // Add to process list
        Lock();
        _processes.push_back(item);
        Unlock();
    }
    else
    {
        Lock();

        // Update the ProcessItem that already Exists
        ProcessItem &item = _processes[index];

        if (!item.active)
        {
            item.active = true;
            item.pid = pi->pid; // The first pid is logged

            _tcscpy_s(item.fullPath, MAX_PATH, pi->fullPath);

            item.txRate = 0;
            item.rxRate = 0;
            item.prevTxRate = 0;
            item.prevRxRate = 0;
        }

        if (pi->dir == DIR_UP)
        {
            item.txRate += pi->size;
        }
        else if (pi->dir == DIR_DOWN)
        {
            item.rxRate += pi->size;
        }
        item.dirty = true;

        Unlock();
    }
}

void ProcessModel::OnTimer()
{
    Lock();

    // Update Process Active State
    bool rebuilt = false;
    for(unsigned int i = 0; i < _processes.size(); i++)
    {
        ProcessItem &item = _processes[i];
        if (item.active && item.pid != -1 ) // Skip the "Unknown" process
        {
            if (!ProcessCache::instance()->IsProcessAlive(item.pid, item.name, !rebuilt))
            {
                item.active = false;
            }
            rebuilt = true;
        }
    }

    // Update Rate
    for(unsigned int i = 0; i < _processes.size(); i++)
    {
        ProcessItem &item = _processes[i];

        item.prevTxRate = item.txRate;
        item.prevRxRate = item.rxRate;

        item.txRate = 0;
        item.rxRate = 0;
    }
    Unlock();

    // Update View
    ProcessView::Update();
}

void ProcessModel::ResetDirty(int puid)
{
    int index = GetProcessIndex(puid);
    Lock();
    _processes[index].dirty = false;
    Unlock();
}

void ProcessModel::ShowProcess(int puid)
{
    // Modify the Model
    int index = GetProcessIndex(puid);
    Lock();
    if (index != -1)
    {
        _processes[index].hidden = false;
    }
    Unlock();

    if (index != -1)
    {
        // Update View
        ProcessView::Update(false, true);

        // Update Profile
        std::vector<int> hiddenProcesses;
        ExportHiddenProcesses(hiddenProcesses);
        g_profile.SetValue(TEXT("HiddenProcess"), new ProfileIntListItem(hiddenProcesses));
    }
}

void ProcessModel::HideProcess(int puid)
{
    // Modify the Model
    int index = GetProcessIndex(puid);
    Lock();
    if (index != -1)
    {
        _processes[index].hidden = true;
    }
    Unlock();

    if (index != -1)
    {
        // Update View
        if (!ProcessView::IsHidden())
        {
            ProcessView::Update(false, true);
        }
        else
        {
            ProcessView::Update(true);
        }

        // Update Profile
        std::vector<int> hiddenProcesses;
        ExportHiddenProcesses(hiddenProcesses);
        g_profile.SetValue(TEXT("HiddenProcess"), new ProfileIntListItem(hiddenProcesses));
    }
}

void ProcessModel::Export(std::vector<ProcessModel::ProcessItem> &items)
{
    Lock();
    items = _processes;
    Unlock();
}

void ProcessModel::ExportHiddenState(std::vector<bool> &states)
{
    Lock();
    states.clear();
    for (unsigned int i = 0; i < _processes.size(); i++)
    {
        states.push_back(_processes[i].hidden);
    }
    Unlock();
}

void ProcessModel::ExportHiddenProcesses(std::vector<int> &processes)
{
    Lock();
    processes.clear();
    for (unsigned int i = 0; i < _processes.size(); i++)
    {
        if (_processes[i].hidden)
        {
            processes.push_back(_processes[i].puid);
        }
    }
    Unlock();
}

// Utils
int ProcessModel::GetProcessUid(int index)
{
    Lock();
    int puid = _processes[index].puid;
    Unlock();
    return puid;
}

int ProcessModel::GetProcessCount()
{
    Lock();
    int size = _processes.size();
    Unlock();
    return size;
}

int ProcessModel::GetProcessUid(const TCHAR *name)
{
    int puid = -1;
    Lock();
    for(unsigned int i = 0; i < _processes.size(); i++)
    {
        if (_tcscmp(_processes[i].name, name) == 0 )
        {
            puid = _processes[i].puid;
            break;
        }
    }
    Unlock();
    return puid;
}

bool ProcessModel::GetProcessName(int puid, TCHAR *buf, int len)
{
    bool result = false;
    Lock();
    for(unsigned int i = 0; i < _processes.size(); i++)
    {
        if (_processes[i].puid == puid )
        {
            _tcscpy_s(buf, len, _processes[i].name);
            result = true;
            break;
        }
    }
    Unlock();
    return result;
}

int ProcessModel::GetProcessIndex(int puid)
{
    int index = -1;
    Lock();
    for(unsigned int i = 0; i < _processes.size(); i++)
    {
        if (_processes[i].puid == puid )
        {
            index = i;
            break;
        }
    }
    Unlock();
    return index;
}

bool ProcessModel::GetProcessRate(int puid, int *txRate, int *rxRate)
{
    int index = GetProcessIndex(puid);

    if (index == -1 )
    {
        return false;
    }
    else
    {
        Lock();
        *txRate = _processes[index].prevTxRate;
        *rxRate = _processes[index].prevRxRate;
        Unlock();
        return true;
    }
}

bool ProcessModel::IsProcessActive(int puid)
{
    bool active = false;
    Lock();
    for(unsigned int i = 0; i < _processes.size(); i++)
    {
        if (_processes[i].puid == puid)
        {
            active = _processes[i].active;
            break;
        }
    }
    Unlock();
    return active;
}