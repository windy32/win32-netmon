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
#include "Utils.h"
#include "ProcessCache.h"

ProcessCache *ProcessCache::_instance = NULL;

ProcessCache::ProcessCache()
{
    InitializeCriticalSection(&_cs);
}

ProcessCache::~ProcessCache()
{
    DeleteCriticalSection(&_cs);
}

ProcessCache *ProcessCache::instance()
{
    if (_instance == NULL)
        _instance = new ProcessCache();
    return _instance;
}

void ProcessCache::GetName(int pid, TCHAR *buf, int cchLen)
{
    EnterCriticalSection(&_cs);

    if (_processTable.count(pid) == 0)
    {
        rebuildTable();
    }
    _tcscpy_s(buf, cchLen, _processTable[pid].name);

    LeaveCriticalSection(&_cs);
}

void ProcessCache::GetFullPath(int pid, TCHAR *buf, int cchLen)
{
    EnterCriticalSection(&_cs);

    if (_processTable.count(pid) == 0)
    {
        rebuildTable();
    }
    _tcscpy_s(buf, cchLen, _processTable[pid].path);

    LeaveCriticalSection(&_cs);
}

BOOL ProcessCache::IsProcessAlive(int pid, const TCHAR *name, bool rebuild)
{
    BOOL result;
    EnterCriticalSection(&_cs);

    if (rebuild)
    {
        rebuildTable(false);
    }
    result = (_processTable.count(pid) > 0 && _tcscmp(_processTable[pid].name, name) == 0) ? 
        TRUE : FALSE;

    LeaveCriticalSection(&_cs);
    return result;
}

void ProcessCache::rebuildTable(bool dump)
{
    // Clear Tables
    _processTable.clear();

    // Take a snapshot
    HANDLE hSnapShot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    PROCESSENTRY32 p;
    p.dwSize = sizeof(PROCESSENTRY32);

    // Traverse Process List
    for(BOOL ret = Process32First(hSnapShot, &p); ret != 0; ret = Process32Next(hSnapShot, &p))
    {
        // Get pid and file name
        int pid = p.th32ProcessID;
        TCHAR *processName = p.szExeFile;

        // Get full path (if possible)
        HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid);
        if (hProcess == 0)
        {
            ProcessInfo info;
            _tcscpy_s(info.name, MAX_PATH, processName);
            _tcscpy_s(info.path, MAX_PATH, _T("-"));
            _processTable[pid] = info;
            if (dump)
            {
                Utils::DbgPrint(_T("   PID = %d, Name = \"%s\", FullPath = \"%s\"\n"), 
                    pid, processName, _T("-"));
            }
        }
        else
        {
            ProcessInfo info;
            _tcscpy_s(info.name, MAX_PATH, processName);

            TCHAR fullPath[MAX_PATH]={ 0 };
            if (GetModuleFileNameEx(hProcess, 0, fullPath, MAX_PATH) > 0) // Success
            {
                _tcscpy_s(info.path, MAX_PATH, fullPath);
            }
            else
            {
                _tcscpy_s(info.path, MAX_PATH, _T("-"));
            }
            _processTable[pid] = info;
        }
        CloseHandle(hProcess);
    }
    CloseHandle(hSnapShot);
}
