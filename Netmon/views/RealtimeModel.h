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

#ifndef REALTIME_MODEL_H
#define REALTIME_MODEL_H

#include "NetModel.h"

enum ZoomFactor
{
    ZOOM_1S,
    ZOOM_10S,
    ZOOM_60S
};

class RealtimeModel : public NetModel
{
private:
    // Item Definition
    typedef struct tagRtModelItem
    {
        std::vector<int> rate_tx_1s;
        std::vector<int> rate_tx_10s;
        std::vector<int> rate_tx_60s;

        std::vector<int> rate_rx_1s;
        std::vector<int> rate_rx_10s;
        std::vector<int> rate_rx_60s;

        struct tagRtModelItem()
        {
            rate_tx_1s.reserve(3600 * 10);
            rate_tx_10s.reserve(360 * 10);
            rate_tx_60s.reserve(60 * 10);

            rate_rx_1s.reserve(3600 * 10);
            rate_rx_10s.reserve(360 * 10);
            rate_rx_60s.reserve(60 * 10);
        }
    } RtModelItem;

    // Items
    std::map<int, RtModelItem> _items;

    // Others
    int _startTime;

private:
    void Fill();

public:
    RealtimeModel();

    // Modify the Model
    void InsertPacket(PacketInfoEx *pi);

    // Export Model Info
    void Export(
        int process, enum ZoomFactor zoom, std::vector<int> &txRate, std::vector<int> &rxRate);
};

#endif
