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
    int timeOffset = (int)time(0) - _startTime;

    unsigned int size_1s  = (unsigned int)(timeOffset + 1);
    unsigned int size_10s = (unsigned int)(timeOffset / 10 + 1);
    unsigned int size_60s = (unsigned int)(timeOffset / 60 + 1);

    // Fill vectors
    Lock();

    for(std::map<int, RtModelItem>::iterator it = _items.begin(); it != _items.end(); ++it)
    {
        while( it->second.rate_tx_1s.size() < size_1s )
        {
            it->second.rate_tx_1s.push_back(0);
            it->second.rate_rx_1s.push_back(0);
        }

        while( it->second.rate_tx_10s.size() < size_10s )
        {
            it->second.rate_tx_10s.push_back(0);
            it->second.rate_rx_10s.push_back(0);
        }

        while( it->second.rate_tx_60s.size() < size_60s )
        {
            it->second.rate_tx_60s.push_back(0);
            it->second.rate_rx_60s.push_back(0);
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
    if( _items.count(pi->puid) == 0 )
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

    if( pi->dir == DIR_UP )
    {
        item.rate_tx_1s.back()  += pi->size;
        item.rate_tx_10s.back() += pi->size;
        item.rate_tx_60s.back() += pi->size;

        itemAll.rate_tx_1s.back()  += pi->size;
        itemAll.rate_tx_10s.back() += pi->size;
        itemAll.rate_tx_60s.back() += pi->size;
    }
    else if( pi->dir == DIR_DOWN )
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
