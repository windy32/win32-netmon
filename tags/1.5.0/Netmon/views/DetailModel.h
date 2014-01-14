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

#ifndef DETAIL_MODEL_H
#define DETAIL_MODEL_H

#include "NetModel.h"

class DetailModel : public NetModel
{
private:
    // Item Definition
    typedef struct tagDtModelItem
    {
        __int64 curPackets;
        __int64 prevPackets;

        struct tagDtModelItem()
        {
            curPackets = 0;
            prevPackets = 0;
        }
    } DtModelItem;

    // Items
    std::map<int, DtModelItem> _items;

private:
    void InitDatabase();

public:
    DetailModel();

    // Modify the Model
    void InsertPacket(PacketInfoEx *pi);
    void SetPrevPackets(int process, __int64 numPackets);
    void ClearPackets();

    // Export Model Info
    __int64 GetCurPackets(int process);
    __int64 GetPrevPackets(int process);

    // Save Model Into to Database
    void SaveDatabase();
};

#endif
