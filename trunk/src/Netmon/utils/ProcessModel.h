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

#ifndef PROCESS_MODEL_H
#define PROCESS_MODEL_H

#include "SQLite.h"
#include "Packet.h"

class ProcessModel
{
public:
    // The Process class maintains a list of process items
    typedef struct tagProcessItem
    {
        // Static Parameters
        bool active;

        int  puid;
        TCHAR name[MAX_PATH];
        TCHAR fullPath[MAX_PATH];

        bool hidden;

        // Valid Only When Active
        bool dirty;

        int txRate;
        int rxRate;
        int prevTxRate;
        int prevRxRate;

        int pid; // Note: A "Process" item may corresponds to multiple pids
    } ProcessItem;

private:
    static std::vector<ProcessItem> _processes;
    static CRITICAL_SECTION _cs;

private:
    static void InitCallback(SQLiteRow *row);
    static void Lock();
    static void Unlock();

public:    
    // Modify the Model
    static void Init();
    static void OnPacket(PacketInfoEx *pi);
    static void OnTimer();

    static void ResetDirty(int puid);
    static void ShowProcess(int puid);
    static void HideProcess(int puid);

    // Export Model Info
    static void Export(std::vector<ProcessItem> &items);
    static void ExportHiddenState(std::vector<bool> &states);
    static void ExportHiddenProcesses(std::vector<int> &processes);

public:
    static int  GetProcessUid(int index);
    static int  GetProcessCount();
    static int  GetProcessUid(const TCHAR *name);
    static bool GetProcessName(int puid, TCHAR *buf, int len);
    static int  GetProcessIndex(int puid);
    static bool GetProcessRate(int puid, int *txRate, int *rxRate);
    static bool IsProcessActive(int puid);
};

#endif
