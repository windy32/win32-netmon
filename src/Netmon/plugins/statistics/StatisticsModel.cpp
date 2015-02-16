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
#include "StatisticsModel.h"
#include "../../Utils/ProcessModel.h"

StatisticsModel::StatisticsModel()
{
    _items[PROCESS_ALL] = StModelItem();
    _clear_flag = false;

    InitDatabase();
    ReadDatabase();
}

StatisticsModel::~StatisticsModel()
{
    if (_clear_flag)
    {
        SQLite::Exec(TEXT("Delete From Protocol;"), true);
        SQLite::Exec(TEXT("Delete From PacketSize;"), true);
        SQLite::Exec(TEXT("Delete From Rate;"), true);
        SQLite::Flush();
        SQLite::Exec(TEXT("Vacuum;"), false);
    }
    else
    {
        SaveDatabase();
    }
}

void StatisticsModel::InitDatabase()
{
    if (!SQLite::TableExist(TEXT("Protocol")))
    {
        SQLite::Exec(TEXT("Create Table Protocol(")
                     TEXT("    ProcessUID     Integer,")
                     TEXT("    Protocol       Integer,")
                     TEXT("    TxBytes        Integer,")
                     TEXT("    RxBytes        Integer,")
                     TEXT("    TxPackets      Integer,")
                     TEXT("    RxPackets      Integer,")
                     TEXT("    ")
                     TEXT("    Primary Key (ProcessUID, Protocol),")
                     TEXT("    Foreign Key (ProcessUID) References Process(UID)")
                     TEXT(");"), true);
    }

    if (!SQLite::TableExist(TEXT("PacketSize")))
    {
        SQLite::Exec(TEXT("Create Table PacketSize(")
                     TEXT("    ProcessUID     Integer,")
                     TEXT("    PacketSize     Integer,")
                     TEXT("    TxBytes        Integer,")
                     TEXT("    RxBytes        Integer,")
                     TEXT("    TxPackets      Integer,")
                     TEXT("    RxPackets      Integer,")
                     TEXT("    ")
                     TEXT("    Primary Key (ProcessUID, PacketSize),")
                     TEXT("    Foreign Key (ProcessUID) References Process(UID)")
                     TEXT(");"), true);
    }

    if (!SQLite::TableExist(TEXT("Rate")))
    {
        SQLite::Exec(TEXT("Create Table Rate(")
                     TEXT("    ProcessUid     Integer,")
                     TEXT("    Rate           Integer,")
                     TEXT("    TxSeconds      Integer,")
                     TEXT("    RxSeconds      Integer,")
                     TEXT("    ")
                     TEXT("    Primary Key (ProcessUID, Rate),")
                     TEXT("    Foreign Key (ProcessUID) References Process(UID)")
                     TEXT(");"), true);
    }
}

void StatisticsModel::ReadDatabase()
{
    SQLiteRow protocolRow;
    SQLiteRow packetSizeRow;
    SQLiteRow rateRow;
    TCHAR command[256];

    // Protocol
    _stprintf_s(command, _countof(command), TEXT("Select * From Protocol;"));

    protocolRow.InsertType(SQLiteRow::TYPE_INT32); // 0 ProcessUid
    protocolRow.InsertType(SQLiteRow::TYPE_INT32); // 1 Protocol
    protocolRow.InsertType(SQLiteRow::TYPE_INT64); // 2 TxBytes
    protocolRow.InsertType(SQLiteRow::TYPE_INT64); // 3 RxBytes
    protocolRow.InsertType(SQLiteRow::TYPE_INT64); // 4 TxPackets
    protocolRow.InsertType(SQLiteRow::TYPE_INT64); // 5 RxPackets

    SQLite::Select(command, &protocolRow, ReadDatabaseProtocolCallback, this);

    // Packet Size
    _stprintf_s(command, _countof(command), 
        TEXT("Select * From PacketSize Where TxPackets > 0 Or RxPackets > 0;"));

    packetSizeRow.InsertType(SQLiteRow::TYPE_INT32); // 0 ProcessUid
    packetSizeRow.InsertType(SQLiteRow::TYPE_INT32); // 1 PacketSize
    packetSizeRow.InsertType(SQLiteRow::TYPE_INT64); // 2 TxBytes
    packetSizeRow.InsertType(SQLiteRow::TYPE_INT64); // 3 RxBytes
    packetSizeRow.InsertType(SQLiteRow::TYPE_INT64); // 4 TxPackets
    packetSizeRow.InsertType(SQLiteRow::TYPE_INT64); // 5 RxPackets

    SQLite::Select(command, &packetSizeRow, ReadDatabasePacketSizeCallback, this);

    // Traffic Rate
    _stprintf_s(command, _countof(command), 
        TEXT("Select * From Rate Where TxSeconds > 0 Or RxSeconds > 0;"));

    rateRow.InsertType(SQLiteRow::TYPE_INT32); // 0 ProcessUid
    rateRow.InsertType(SQLiteRow::TYPE_INT32); // 1 Rate
    rateRow.InsertType(SQLiteRow::TYPE_INT64); // 2 TxSeconds
    rateRow.InsertType(SQLiteRow::TYPE_INT64); // 3 RxSeconds

    SQLite::Select(command, &rateRow, ReadDatabaseRateCallback, this);
}

void StatisticsModel::ReadDatabaseProtocolCallback(SQLiteRow *row, void *context)
{
    StatisticsModel *model = (StatisticsModel *)context;

    int puid          = row->GetDataInt32(0);
    int protocol      = row->GetDataInt32(1); // Higher 16 bit for exMonth (Jan 1970 = 0), 
                                              // lower 16 bit for mday
    __int64 txBytes   = row->GetDataInt64(2);
    __int64 rxBytes   = row->GetDataInt64(3);
    __int64 txPackets = row->GetDataInt64(4);
    __int64 rxPackets = row->GetDataInt64(5);

    // Insert an StViewItem if PUID not Exist
    if (model->_items.count(puid) == 0 )
    {
        model->_items[puid] = StModelItem();
    }

    // Update
    StModelItem &sItem = model->_items[puid];
    StModelItem &sItemAll = model->_items[PROCESS_ALL];

    switch ( protocol )
    {
    case TRA_TCP:
        sItem.tx.tcpBytes = txBytes;
        sItem.rx.tcpBytes = rxBytes;
        sItem.tx.tcpPackets = txPackets;
        sItem.rx.tcpPackets = rxPackets;

        sItemAll.tx.tcpBytes += txBytes;
        sItemAll.rx.tcpBytes += rxBytes;
        sItemAll.tx.tcpPackets += txPackets;
        sItemAll.rx.tcpPackets += rxPackets;
        break;

    case TRA_UDP:
        sItem.tx.udpBytes = txBytes;
        sItem.rx.udpBytes = rxBytes;
        sItem.tx.udpPackets = txPackets;
        sItem.rx.udpPackets = rxPackets;

        sItemAll.tx.udpBytes += txBytes;
        sItemAll.rx.udpBytes += rxBytes;
        sItemAll.tx.udpPackets += txPackets;
        sItemAll.rx.udpPackets += rxPackets;
        break;

    case TRA_ICMP:
        sItem.tx.icmpBytes = txBytes;
        sItem.rx.icmpBytes = rxBytes;
        sItem.tx.icmpPackets = txPackets;
        sItem.rx.icmpPackets = rxPackets;

        sItemAll.tx.icmpBytes += txBytes;
        sItemAll.rx.icmpBytes += rxBytes;
        sItemAll.tx.icmpPackets += txPackets;
        sItemAll.rx.icmpPackets += rxPackets;
        break;

    case TRA_OTHER:
        sItem.tx.otherBytes = txBytes;
        sItem.rx.otherBytes = rxBytes;
        sItem.tx.otherPackets = txPackets;
        sItem.rx.otherPackets = rxPackets;

        sItemAll.tx.otherBytes += txBytes;
        sItemAll.rx.otherBytes += rxBytes;
        sItemAll.tx.otherPackets += txPackets;
        sItemAll.rx.otherPackets += rxPackets;
        break;

    default:
        break;
    }
}

void StatisticsModel::ReadDatabasePacketSizeCallback(SQLiteRow *row, void *context)
{
    StatisticsModel *model = (StatisticsModel *)context;

    int puid          = row->GetDataInt32(0);
    int packetSize    = row->GetDataInt32(1); // [0, 1500]: 1 to 1501+
    __int64 txBytes   = row->GetDataInt64(2);
    __int64 rxBytes   = row->GetDataInt64(3);
    __int64 txPackets = row->GetDataInt64(4);
    __int64 rxPackets = row->GetDataInt64(5);

    // Insert an StViewItem if PUID not Exist
    if (model->_items.count(puid) == 0 )
    {
        model->_items[puid] = StModelItem();
    }

    // Update
    StModelItem &sItem = model->_items[puid];
    StModelItem &sItemAll = model->_items[PROCESS_ALL];

    sItem.txPacketSize[packetSize] = txPackets;
    sItem.rxPacketSize[packetSize] = rxPackets;
    sItem.txPrevPacketSize[packetSize] = txPackets;
    sItem.rxPrevPacketSize[packetSize] = rxPackets;

    sItemAll.txPacketSize[packetSize] += txPackets;
    sItemAll.rxPacketSize[packetSize] += rxPackets;
    sItemAll.txPrevPacketSize[packetSize] += txPackets;
    sItemAll.rxPrevPacketSize[packetSize] += rxPackets;
}

void StatisticsModel::ReadDatabaseRateCallback(SQLiteRow *row, void *context)
{
    StatisticsModel *model = (StatisticsModel *)context;

    int puid          = row->GetDataInt32(0);
    int rate          = row->GetDataInt32(1); // [0, 1024]: 0 to 1024+
    __int64 txSeconds = row->GetDataInt64(2);
    __int64 rxSeconds = row->GetDataInt64(3);

    // Insert an StViewItem if PUID not Exist
    if (model->_items.count(puid) == 0 )
    {
        model->_items[puid] = StModelItem();
    }

    // Update
    StModelItem &sItem = model->_items[puid];
    StModelItem &sItemAll = model->_items[PROCESS_ALL];

    sItem.txRate[rate] = txSeconds;
    sItem.rxRate[rate] = rxSeconds;
    sItem.txPrevRate[rate] = txSeconds;
    sItem.rxPrevRate[rate] = rxSeconds;

    sItemAll.txRate[rate] += txSeconds;
    sItemAll.rxRate[rate] += rxSeconds;
    sItemAll.txPrevRate[rate] += txSeconds;
    sItemAll.rxPrevRate[rate] += rxSeconds;
}

void StatisticsModel::SaveDatabase()
{
    TCHAR command[256];

    Lock();

    // Delete all records - Protocol
    SQLite::Exec(TEXT("Delete From Protocol;"), true);

    // Insert records - Protocol
    std::map<int, StModelItem>::iterator it;
    for(it = _items.begin(); it != _items.end(); ++it) // Loop of Process
    {
        int puid = it->first;

        if (puid == PROCESS_ALL )
        {
            continue;
        }

        // TCP
        _stprintf_s(command, _countof(command), 
            TEXT("Insert Into Protocol Values(%d, %d, %I64d, %I64d, %I64d, %I64d);"), 
            puid, TRA_TCP, 
            it->second.tx.tcpBytes, 
            it->second.rx.tcpBytes, 
            it->second.tx.tcpPackets, 
            it->second.rx.tcpPackets);
        SQLite::Exec(command, true);

        // UDP
        _stprintf_s(command, _countof(command), 
            TEXT("Insert Into Protocol Values(%d, %d, %I64d, %I64d, %I64d, %I64d);"), 
            puid, TRA_UDP, 
            it->second.tx.udpBytes, 
            it->second.rx.udpBytes, 
            it->second.tx.udpPackets, 
            it->second.rx.udpPackets);
        SQLite::Exec(command, true);

        // ICMP
        _stprintf_s(command, _countof(command), 
            TEXT("Insert Into Protocol Values(%d, %d, %I64d, %I64d, %I64d, %I64d);"), 
            puid, TRA_ICMP, 
            it->second.tx.icmpBytes, 
            it->second.rx.icmpBytes, 
            it->second.tx.icmpPackets, 
            it->second.rx.icmpPackets);
        SQLite::Exec(command, true);

        // OTHER
        _stprintf_s(command, _countof(command), 
            TEXT("Insert Into Protocol Values(%d, %d, %I64d, %I64d, %I64d, %I64d);"), 
            puid, TRA_OTHER, 
            it->second.tx.otherBytes, 
            it->second.rx.otherBytes, 
            it->second.tx.otherPackets, 
            it->second.rx.otherPackets);
        SQLite::Exec(command, true);
    }

    // Insert or update records - PacketSize
    for(it = _items.begin(); it != _items.end(); ++it) // Loop of Process
    {
        int puid = it->first;

        if (puid == PROCESS_ALL )
        {
            continue;
        }

        if (it->second.newItem == true ) // Insert
        {
            for(int i = 0; i < 1501; i++)
            {
                _stprintf_s(command, _countof(command), 
                    TEXT("Insert Into PacketSize Values(%d, %d, 0, 0, %I64d, %I64d);"), 
                    puid, i, it->second.txPacketSize[i], it->second.rxPacketSize[i]);

                SQLite::Exec(command, true);
            }
        }
        else // Update
        {
            for(int i = 0; i < 1501; i++)
            {
                if (it->second.txPacketSize[i] > it->second.txPrevPacketSize[i] ||
                    it->second.rxPacketSize[i] > it->second.rxPrevPacketSize[i] )
                {
                    _stprintf_s(command, _countof(command), 
                        TEXT("Update PacketSize Set TxPackets = %I64d, RxPackets = %I64d ")
                        TEXT("Where ProcessUid = %d And PacketSize = %d;"), 
                        it->second.txPacketSize[i], it->second.rxPacketSize[i], puid, i);

                    SQLite::Exec(command, true);
                }
            }
        }
    }

    // Insert or update records - Rate
    for(it = _items.begin(); it != _items.end(); ++it) // Loop of Process
    {
        int puid = it->first;

        if (puid == PROCESS_ALL )
        {
            continue;
        }

        if (it->second.newItem == true ) // Insert
        {
            for(int i = 0; i < 1025; i++)
            {
                _stprintf_s(command, _countof(command), 
                    TEXT("Insert Into Rate Values(%d, %d, %I64d, %I64d);"), 
                    puid, i, it->second.txRate[i], it->second.rxRate[i]);

                SQLite::Exec(command, true);
            }
        }
        else // Update
        {
            for(int i = 0; i < 1025; i++)
            {
                if (it->second.txRate[i] > it->second.txPrevRate[i] ||
                    it->second.rxRate[i] > it->second.rxPrevRate[i] )
                {
                    _stprintf_s(command, _countof(command), 
                        TEXT("Update Rate Set TxSeconds = %I64d, RxSeconds = %I64d ")
                        TEXT("Where ProcessUid = %d And Rate = %d;"), 
                        it->second.txRate[i], it->second.rxRate[i], puid, i);

                    SQLite::Exec(command, true);
                }
            }
        }
    }

    Unlock();

    // Flush
    SQLite::Flush();
}

void StatisticsModel::ClearDatabase()
{
    _clear_flag = true;
}

void StatisticsModel::InsertPacket(PacketInfoEx *pi)
{
    Lock();

    // Insert a StViewItem if PUID not Exist
    if (_items.count(pi->puid) == 0 )
    {
        _items[pi->puid] = StModelItem();
        _items[pi->puid].newItem = true;
    }

    // Create a Reference for Short
    StModelItem &item = _items[pi->puid];
    StModelItem &itemAll = _items[PROCESS_ALL];

    // Update Statistics
    if (pi->dir == DIR_UP )
    {
        switch ( pi->trasportProtocol )
        {
        case TRA_TCP:
            item.tx.tcpBytes += pi->size;
            itemAll.tx.tcpBytes += pi->size;
            break;

        case TRA_UDP:
            item.tx.udpBytes += pi->size;
            itemAll.tx.udpBytes += pi->size;
            break;

        case TRA_ICMP:
            item.tx.icmpBytes += pi->size;
            itemAll.tx.icmpBytes += pi->size;
            break;

        case TRA_OTHER:
            item.tx.otherBytes += pi->size;
            itemAll.tx.otherBytes += pi->size;
            break;

        default:
            break;
        }

        // Set Size Range to [1, 1501]
        int size = pi->size;

        size = ( size <= 1 ) ? 1 : size;
        size = ( size >= 1501 ) ? 1501 : size;

        item.txPacketSize[size - 1] += 1;
        itemAll.txPacketSize[size - 1] += 1;
    }
    else if (pi->dir == DIR_DOWN )
    {
        switch ( pi->trasportProtocol )
        {
        case TRA_TCP:
            item.rx.tcpBytes += pi->size;
            itemAll.rx.tcpBytes += pi->size;
            break;

        case TRA_UDP:
            item.rx.udpBytes += pi->size;
            itemAll.rx.udpBytes += pi->size;
            break;

        case TRA_ICMP:
            item.rx.icmpBytes += pi->size;
            itemAll.rx.icmpBytes += pi->size;
            break;

        case TRA_OTHER:
            item.rx.otherBytes += pi->size;
            itemAll.rx.otherBytes += pi->size;
            break;

        default:
            break;
        }

        // Set Size Range to [1, 1501]
        int size = pi->size;

        size = ( size <= 1 ) ? 1 : size;
        size = ( size >= 1501 ) ? 1501 : size;

        item.rxPacketSize[size - 1] += 1;
        itemAll.rxPacketSize[size - 1] += 1;
    }

    Unlock();
}

void StatisticsModel::UpdateRate()
{
    Lock();

    // Update statistics for rate
    for(std::map<int, StModelItem>::iterator it = _items.begin(); it != _items.end(); ++it)
    {
        int puid = it->first;

        if (puid == PROCESS_ALL )
        {
            continue;
        }

        StModelItem &item = _items[puid];
        StModelItem &itemAll = _items[PROCESS_ALL];
        int txRate;
        int rxRate;

        if (ProcessModel::IsProcessActive(puid))
        {
            ProcessModel::GetProcessRate(puid, &txRate, &rxRate);

            txRate /= 1024; // Unit KB/s
            rxRate /= 1024; 

            txRate = txRate > 1024 ? 1024 : txRate; // Limit 1024 KB/s
            rxRate = rxRate > 1024 ? 1024 : rxRate;

            item.txRate[txRate] += 1;
            item.rxRate[rxRate] += 1;

            itemAll.txRate[txRate] += 1;
            itemAll.rxRate[rxRate] += 1;
        }
    }

    Unlock();
}

void StatisticsModel::Export(int process, StModelItem &item)
{
    Lock();
    if (_items.count(process) != 0)
    {
        item = _items[process];
    }
    Unlock();
};
