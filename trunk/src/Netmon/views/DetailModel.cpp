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
#include "DetailModel.h"
#include "../Utils/ProcessModel.h"
#include "../Utils/Utils.h"

DetailModel::DetailModel()
{
    _items[PROCESS_ALL] = DtModelItem();
    InitDatabase();
}

void DetailModel::InitDatabase()
{
    int puid;
    TCHAR name[256];
    __int64 packetCount;
    TCHAR command[256];

    for(int i = 0; i < ProcessModel::GetProcessCount(); i++)
    {
        // Get puid, name and packetCount
        puid = ProcessModel::GetProcessUid(i);
        ProcessModel::GetProcessName(puid, name, _countof(name));

        _stprintf_s(command, _countof(command), 
            TEXT("Select Count From PacketCount Where ProcessUid = \'%d\';"), puid);

        SQLiteRow row;
        row.InsertType(SQLiteRow::TYPE_INT64); // 0 Count(*)
        if( SQLite::Select(command, &row))
        {
            packetCount = row.GetDataInt64(0);
        }
        else
        {
            packetCount = 0;
        }

        // Insert DtViewItem
        _items[puid] = DtModelItem();
        _items[puid].curPackets = packetCount;

        // Update corresponding PROCESS_ALL item
        _items[PROCESS_ALL].curPackets += packetCount;
    }
}

void DetailModel::SaveDatabase()
{
    Lock();

    // Delete all records
    SQLite::Exec(TEXT("Delete From PacketCount;"), true);

    // Insert records
    for(std::map<int, DtModelItem>::iterator it = _items.begin(); it != _items.end(); ++it)
    {
        int puid = it->first;
        __int64 count = it->second.curPackets;

        if( puid == PROCESS_ALL )
        {
            continue;
        }

        // Build Command
        TCHAR command[256];
        _stprintf_s(command, _countof(command), 
            TEXT("Insert Into PacketCount Values(%d, %I64d);"), puid, count);

        // Insert
        SQLite::Exec(command, true);
    }

    Unlock();

    // Flush
    SQLite::Flush();
}


void DetailModel::InsertPacket(PacketInfoEx *pi)
{
    Lock();

    // Insert an DtViewItem if PUID not Exist
    if( _items.count(pi->puid) == 0 )
    {
        _items[pi->puid] = DtModelItem();
    }

    // Update packet count
    _items[pi->puid].curPackets += 1;
    _items[PROCESS_ALL].curPackets += 1;

    Unlock();
}

void DetailModel::SetPrevPackets(int process, __int64 numPackets)
{
    if (_items.count(process) != 0)
        _items[process].prevPackets = numPackets;
}

void DetailModel::ClearPackets()
{
    for(std::map<int, DtModelItem>::iterator it = _items.begin(); it != _items.end(); ++it)
    {
        it->second.curPackets = 0;
    }
}

__int64 DetailModel::GetCurPackets(int process)
{
    if (_items.count(process) != 0)
        return _items[process].curPackets;
    else
        return 0;
}

__int64 DetailModel::GetPrevPackets(int process)
{
    if (_items.count(process) != 0)
        return _items[process].prevPackets;
    else
        return 0;
}
