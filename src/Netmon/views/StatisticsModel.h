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

#ifndef STATISTICS_MODEL_H
#define STATISTICS_MODEL_H

#include "NetModel.h"
#include "../Utils/SQLite.h"

class StatisticsModel : public NetModel
{
public:
    // Item Definition
    typedef struct tagStModelItem
    {
        struct tagStCountItem
        {
            __int64 tcpPackets;
            __int64 udpPackets;
            __int64 icmpPackets;
            __int64 otherPackets;

            __int64 tcpBytes;
            __int64 udpBytes;
            __int64 icmpBytes;
            __int64 otherBytes;
        } rx, tx;

        __int64 rxPacketSize[1501]; // 1 to 1501+ Bytes
        __int64 txPacketSize[1501];

        __int64 rxPrevPacketSize[1501];
        __int64 txPrevPacketSize[1501];

        __int64 rxRate[1025]; // 0 to 1024+ KB/s
        __int64 txRate[1025];

        __int64 rxPrevRate[1025];
        __int64 txPrevRate[1025];

        bool newItem; // false when the StViewItem is created in InitDatabase

        struct tagStModelItem()
        {
            RtlZeroMemory(&rx, sizeof(rx));
            RtlZeroMemory(&tx, sizeof(tx));

            RtlZeroMemory(rxPacketSize, sizeof(rxPacketSize));
            RtlZeroMemory(txPacketSize, sizeof(txPacketSize));

            RtlZeroMemory(rxPrevPacketSize, sizeof(rxPrevPacketSize));
            RtlZeroMemory(txPrevPacketSize, sizeof(txPrevPacketSize));

            RtlZeroMemory(rxRate, sizeof(rxRate));
            RtlZeroMemory(txRate, sizeof(txRate));

            RtlZeroMemory(rxPrevRate, sizeof(rxPrevRate));
            RtlZeroMemory(txPrevRate, sizeof(txPrevRate));

            newItem = false;
        }
    } StModelItem;

private:
    // Items
    std::map<int, StModelItem> _items;

    // Others
    static StatisticsModel *_this;

private:
    void InitDatabase();
    static void InitDatabaseProtocolCallback(SQLiteRow *row);
    static void InitDatabasePacketSizeCallback(SQLiteRow *row);
    static void InitDatabaseRateCallback(SQLiteRow *row);

public:
    StatisticsModel();

    // Modify the Model
    void InsertPacket(PacketInfoEx *pi);
    void UpdateRate(); // Called once a second

    // Export Model Info
    void Export(int process, StModelItem &item);

    // Save Model Into to Database
    void SaveDatabase();
};

#endif
