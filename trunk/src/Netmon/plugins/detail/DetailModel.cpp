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
#include "../../Utils/ProcessModel.h"
#include "../../Utils/Utils.h"

DetailModel::DetailModel()
{
    _packetCounts[PROCESS_ALL] = 0;

    InitDatabase();
    ReadDatabase();
}

DetailModel::~DetailModel()
{
    SaveDatabase();
}

void DetailModel::InitDatabase()
{
    if (!SQLite::TableExist(TEXT("Packet")))
    {
        SQLite::Exec(TEXT("Create Table Packet(")
                     TEXT("    UID            Integer,")
                     TEXT("    ProcessUID     Integer,")
                     TEXT("    Direction      Integer,")
                     TEXT("    NetProtocol    Integer,")
                     TEXT("    TraProtocol    Integer,")
                     TEXT("    Size           Integer,")
                     TEXT("    Time           Integer,")
                     TEXT("    Port           Integer,")
                     TEXT("    ")
                     TEXT("    Primary Key (UID),")
                     TEXT("    Foreign Key (ProcessUID) References Process(UID)")
                     TEXT(");"), true);

        SQLite::Exec(TEXT("Create Index PUID On Packet(ProcessUID);"), true);
    }

    if (!SQLite::TableExist(TEXT("PacketCount")))
    {
        SQLite::Exec(TEXT("Create Table PacketCount(")
                     TEXT("    ProcessUID     Integer,")
                     TEXT("    Count          Integer,")
                     TEXT("    ")
                     TEXT("    Primary Key (ProcessUID),")
                     TEXT("    Foreign Key (ProcessUID) References Process(UID)")
                     TEXT(");"), true);
    }
}

void DetailModel::ReadDatabase()
{
    TCHAR name[256];
    TCHAR command[256];

    for(int i = 0; i < ProcessModel::GetProcessCount(); i++)
    {
        // Get puid, name and packetCount
        int puid = ProcessModel::GetProcessUid(i);
        ProcessModel::GetProcessName(puid, name, _countof(name));

        _stprintf_s(command, _countof(command), 
            TEXT("Select Count From PacketCount Where ProcessUid = \'%d\';"), puid);

        SQLiteRow row;
        row.InsertType(SQLiteRow::TYPE_INT64); // 0 Count(*)
        __int64 packetCount = 
            SQLite::Select(command, &row) ? row.GetDataInt64(0) : 0;

        // Insert Packet Count
        _packetCounts[puid] = packetCount;

        // Update corresponding PROCESS_ALL item
        _packetCounts[PROCESS_ALL] += packetCount;
    }
}

void DetailModel::SaveDatabase()
{
    Lock();

    // Delete all records
    SQLite::Exec(TEXT("Delete From PacketCount;"), true);

    // Insert records
    for(std::map<int, __int64>::iterator it = _packetCounts.begin(); it != _packetCounts.end(); ++it)
    {
        int puid = it->first;
        __int64 count = it->second;

        if (puid == PROCESS_ALL )
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

void DetailModel::ClearDatabase()
{
    SQLite::Exec(TEXT("Delete From Packet;"), true);
    SQLite::Exec(TEXT("Delete From PacketCount;"), true);
}

void DetailModel::InsertPacket(PacketInfoEx *pi)
{
    Lock();

    // Add to database
    Utils::InsertPacket(pi);

    // Insert a value if PUID not Exist
    if (_packetCounts.count(pi->puid) == 0)
    {
        _packetCounts[pi->puid] = 0;
    }

    // Update packet count
    _packetCounts[pi->puid] += 1;
    _packetCounts[PROCESS_ALL] += 1;

    Unlock();
}

void DetailModel::Export(int process, __int64 page, std::vector<PacketItem> &packets)
{
    TCHAR command[256];
    if (process == PROCESS_ALL)
    {
        _stprintf_s(command, _countof(command), 
            TEXT("Select * From Packet Limit 100 Offset %I64d;"), page * 100);
    }
    else
    {
        _stprintf_s(command, _countof(command), 
            TEXT("Select * From Packet Where ProcessUID = %d Limit 100 Offset %I64d;"), 
            process, page * 100);
    }

    SQLiteRow row;
    row.InsertType(SQLiteRow::TYPE_INT32); // 0 UID
    row.InsertType(SQLiteRow::TYPE_INT32); // 1 ProcessUID
    row.InsertType(SQLiteRow::TYPE_INT32); // 2 Direction
    row.InsertType(SQLiteRow::TYPE_INT32); // 3 NetProtocol
    row.InsertType(SQLiteRow::TYPE_INT32); // 4 TraProtocol
    row.InsertType(SQLiteRow::TYPE_INT32); // 5 Size
    row.InsertType(SQLiteRow::TYPE_INT64); // 6 Time
    row.InsertType(SQLiteRow::TYPE_INT32); // 7 Port

    SQLite::Select(command, &row, ExportCallback, &packets);
}

void DetailModel::ExportCallback(SQLiteRow *row, void *context)
{
    std::vector<PacketItem> *packets = (std::vector<PacketItem> *)context;

    PacketItem p;

    p.uid      = row->GetDataInt32(0);
    p.puid     = row->GetDataInt32(1);
    p.dir      = row->GetDataInt32(2);
    p.protocol = row->GetDataInt32(4);
    p.size     = row->GetDataInt32(5);
    p.time     = row->GetDataInt64(6);
    p.port     = row->GetDataInt32(7);

    packets->push_back(p);
}

__int64 DetailModel::GetFirstPageIndex(int puid)
{
    return 0;
}

__int64 DetailModel::GetLastPageIndex(int puid)
{
    if (_packetCounts.count(puid) == 0)
    {
        return 0;
    }
    return (_packetCounts[puid] - 1) / 100;
}

__int64 DetailModel::GetPacketCount(int puid)
{
    if (_packetCounts.count(puid) == 0)
    {
        return 0;
    }
    return _packetCounts[puid];
}
