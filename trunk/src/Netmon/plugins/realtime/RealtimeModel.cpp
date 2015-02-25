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
#include "RealtimeModel.h"

RealtimeModel::RealtimeModel()
{
    _startTime = (int)time(0);
    _items[PROCESS_ALL] = RtModelItem();
}

void RealtimeModel::Fill()
{
    // Calculate the desired length of each vectors
    // 
    // e.g.: When _startTime = 500
    //
    // Packets with time ranging from 500 to 509 is added to rate_10s[0]
    // Packets with time ranging form 510 to 519 is added to rate_10s[1]
    // ...
    // When time timeOffset < 10, there should be one element in rate_10s
    // When time timeOffset < 20, there should be two element in rate_20s
    // ...

    // If the system time is reset, it will crash here, so we have to fix
    if (time(0) < _startTime)
    {
        // Reset start time
        _startTime = (int)time(0);

        // Clear all items
        for(std::map<int, RtModelItem>::iterator it = _items.begin(); it != _items.end(); ++it)
        {
            RtModelItem &item = it->second;

            item.rate_tx_1s.clear();
            item.rate_tx_10s.clear();
            item.rate_tx_60s.clear();

            item.rate_rx_1s.clear();
            item.rate_rx_10s.clear();
            item.rate_rx_60s.clear();

            item.removed_1s = 0;
            item.removed_10s = 0;
            item.removed_60s = 0;
        }
    }

    int timeOffset = (int)time(0) - _startTime;

    unsigned int size_1s  = (unsigned int)(timeOffset + 1);
    unsigned int size_10s = (unsigned int)(timeOffset / 10 + 1);
    unsigned int size_60s = (unsigned int)(timeOffset / 60 + 1);

    // Fill vectors
    Lock();

    for(std::map<int, RtModelItem>::iterator it = _items.begin(); it != _items.end(); ++it)
    {
        RtModelItem &item = it->second;

        if (item.rate_tx_1s.size() > 8 * 1024) // Remove at least 4 KB one time
        {
            item.rate_tx_1s.erase(item.rate_tx_1s.begin(), item.rate_tx_1s.begin() + 4 * 1024);
            item.rate_rx_1s.erase(item.rate_rx_1s.begin(), item.rate_rx_1s.begin() + 4 * 1024);
            item.removed_1s += 4 * 1024;
        }
        while (item.rate_tx_1s.size() < size_1s - item.removed_1s)
        {
            item.rate_tx_1s.push_back(0);
            item.rate_rx_1s.push_back(0);
        }

        if (item.rate_tx_10s.size() > 8 * 1024)
        {
            item.rate_tx_10s.erase(item.rate_tx_10s.begin(), item.rate_tx_10s.begin() + 4 * 1024);
            item.rate_rx_10s.erase(item.rate_rx_10s.begin(), item.rate_rx_10s.begin() + 4 * 1024);
            item.removed_10s += 4 * 1024;
        }
        while ( item.rate_tx_10s.size() < size_10s - item.removed_10s)
        {
            item.rate_tx_10s.push_back(0);
            item.rate_rx_10s.push_back(0);
        }

        if (item.rate_tx_60s.size() > 8 * 1024)
        {
            item.rate_tx_60s.erase(item.rate_tx_60s.begin(), item.rate_tx_60s.begin() + 4 * 1024);
            item.rate_rx_60s.erase(item.rate_rx_60s.begin(), item.rate_rx_60s.begin() + 4 * 1024);
            item.removed_60s += 4 * 1024;
        }

        while (item.rate_tx_60s.size() < size_60s - item.removed_60s)
        {
            item.rate_tx_60s.push_back(0);
            item.rate_rx_60s.push_back(0);
        }
    }

    Unlock();
}

void RealtimeModel::InsertPacket(PacketInfoEx *pi)
{
    // One day is 86400 seconds, each second 4 bytes, which sums up to be 337.5KB.
    // 10 processed with tx/rx rate: 6.75MB

    // Insert a RtModelItem if PUID not Exist
    Lock();
    if (_items.count(pi->puid) == 0 )
    {
        _items[pi->puid] = RtModelItem();
    }
    Unlock();

    // Fill Vectors
    Fill();

    // Add the Packet's Size to Vectors
    Lock();
    RtModelItem &item = _items[pi->puid];
    RtModelItem &itemAll = _items[PROCESS_ALL];

    if (pi->dir == DIR_UP )
    {
        item.rate_tx_1s.back()  += pi->size;
        item.rate_tx_10s.back() += pi->size;
        item.rate_tx_60s.back() += pi->size;

        itemAll.rate_tx_1s.back()  += pi->size;
        itemAll.rate_tx_10s.back() += pi->size;
        itemAll.rate_tx_60s.back() += pi->size;
    }
    else if (pi->dir == DIR_DOWN )
    {
        item.rate_rx_1s.back()  += pi->size;
        item.rate_rx_10s.back() += pi->size;
        item.rate_rx_60s.back() += pi->size;

        itemAll.rate_rx_1s.back()  += pi->size;
        itemAll.rate_rx_10s.back() += pi->size;
        itemAll.rate_rx_60s.back() += pi->size;
    }
    Unlock();
}

void RealtimeModel::Export(
    int process, enum ZoomFactor zoom, std::vector<int> &txRate, std::vector<int> &rxRate)
{
    Fill();
    Lock();

    if (_items.count(process) != 0)
    {
        if (zoom == ZOOM_1S)
        {
            txRate = _items[process].rate_tx_1s;
            rxRate = _items[process].rate_rx_1s;
        }
        else if (zoom == ZOOM_10S)
        {
            txRate = _items[process].rate_tx_10s;
            rxRate = _items[process].rate_rx_10s;
        }
        else // ZOOM_60S
        {
            txRate = _items[process].rate_tx_60s;
            rxRate = _items[process].rate_rx_60s;
        }
    }
    Unlock();
};
