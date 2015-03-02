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

    // If the system time is moved backward, it will crash here, so we have to fix
    if (time(0) < _startTime)
    {
        Lock();

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
        Unlock();
    }

    int timeOffset = (int)time(0) - _startTime; // now timeOffset >= 0

    int size_1s  = timeOffset + 1;
    int size_10s = timeOffset / 10 + 1;
    int size_60s = timeOffset / 60 + 1;

    // Fill vectors
    Lock();

    for(std::map<int, RtModelItem>::iterator it = _items.begin(); it != _items.end(); ++it)
    {
        RtModelItem &item = it->second;

        // If the system time is moved forward too much, timeOffset will be very large
        // In this case, there's no need to first push_back all the items and then erase them
        //              -------------- new_size ---------------
        //             /                                       \
        //            /                  ------- add_size -------
        //           /  current vector  /                        \
        // +--------o==================o--------------------------+
        // ^        ^                  ^                          ^
        // 0        removed            current_size        new_size (e.g., size_1s)
        int add_size_1s  = size_1s  - item.removed_1s  - (int)item.rate_rx_1s.size();
        int add_size_10s = size_10s - item.removed_10s - (int)item.rate_rx_10s.size();
        int add_size_60s = size_60s - item.removed_60s - (int)item.rate_rx_60s.size();

        if (add_size_1s > 8 * 1024) // avoid add_size too large
        {
            item.rate_tx_1s.clear();
            item.rate_rx_1s.clear();
            item.rate_tx_1s.resize(4 * 1024, 0);
            item.rate_rx_1s.resize(4 * 1024, 0);
            item.removed_1s = size_1s - 4 * 1024;
        }
        else // push_back
        {
            item.rate_tx_1s.insert(item.rate_tx_1s.end(), add_size_1s, 0);
            item.rate_rx_1s.insert(item.rate_rx_1s.end(), add_size_1s, 0);

            while (item.rate_tx_1s.size() > 8 * 1024)
            {
                item.rate_tx_1s.erase(item.rate_tx_1s.begin(), item.rate_tx_1s.begin() + 4 * 1024);
                item.rate_rx_1s.erase(item.rate_rx_1s.begin(), item.rate_rx_1s.begin() + 4 * 1024);
            }
        }

        if (add_size_10s > 8 * 1024) // avoid add_size too large
        {
            item.rate_tx_10s.clear();
            item.rate_rx_10s.clear();
            item.rate_tx_10s.resize(4 * 1024, 0);
            item.rate_rx_10s.resize(4 * 1024, 0);
            item.removed_10s = size_10s - 4 * 1024;
        }
        else // push_back
        {
            item.rate_tx_10s.insert(item.rate_tx_10s.end(), add_size_10s, 0);
            item.rate_rx_10s.insert(item.rate_rx_10s.end(), add_size_10s, 0);

            while (item.rate_tx_10s.size() > 8 * 1024)
            {
                item.rate_tx_10s.erase(item.rate_tx_10s.begin(), item.rate_tx_10s.begin() + 4 * 1024);
                item.rate_rx_10s.erase(item.rate_rx_10s.begin(), item.rate_rx_10s.begin() + 4 * 1024);
            }
        }

        if (add_size_60s > 8 * 1024) // avoid add_size too large
        {
            item.rate_tx_60s.clear();
            item.rate_rx_60s.clear();
            item.rate_tx_60s.resize(4 * 1024, 0);
            item.rate_rx_60s.resize(4 * 1024, 0);
            item.removed_60s = size_60s - 4 * 1024;
        }
        else // push_back, and then remove_front
        {
            item.rate_tx_60s.insert(item.rate_tx_60s.end(), add_size_60s, 0);
            item.rate_rx_60s.insert(item.rate_rx_60s.end(), add_size_60s, 0);

            while (item.rate_tx_60s.size() > 8 * 1024)
            {
                item.rate_tx_60s.erase(item.rate_tx_60s.begin(), item.rate_tx_60s.begin() + 4 * 1024);
                item.rate_rx_60s.erase(item.rate_rx_60s.begin(), item.rate_rx_60s.begin() + 4 * 1024);
            }
        }
    }

    Unlock();
}

void RealtimeModel::InsertPacket(PacketInfoEx *pi)
{
    // Each process uses about 4k * 6 * 4B = 96 KB memoory
    // 100 processes use about 9.6 MB

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
