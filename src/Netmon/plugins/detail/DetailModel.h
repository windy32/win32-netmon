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

#include "../abstract/Model.h"
#include "../../Utils/SQLite.h"

class DetailModel : public Model
{
public:
    // Item Definition
    typedef struct tagPacketItem // Packet detail which is not stored in memory
    {
        int uid;
        int puid;
        int dir;
        int protocol;
        int size;
        __int64 time;
        int port;

        bool operator==(const struct tagPacketItem &right)
        {
            return
                (this->uid == right.uid) &&
                (this->puid == right.puid) &&
                (this->dir == right.dir) &&
                (this->protocol == right.protocol) &&
                (this->size == right.size) &&
                (this->time == right.time) &&
                (this->port == right.port);
        }

        bool operator!=(const struct tagPacketItem &right)
        {
            return !operator==(right);
        }
    } PacketItem;

private:
    // Packet Count which is stored in database
    std::map<int, __int64> _packetCounts;
    bool _clear_flag;

private:
    void InitDatabase();
    void ReadDatabase();
    static void ExportCallback(SQLiteRow *row, void *context);

public:
    DetailModel();
    ~DetailModel();

    // Modify the Model
    void InsertPacket(PacketInfoEx *pi);

    // Export Model Info
    void Export(int process, __int64 page, std::vector<PacketItem> &packets);
    
    // Get Page Indexes
    __int64 GetFirstPageIndex(int puid);
    __int64 GetLastPageIndex(int puid);

    // Get Packet Count
    __int64 GetPacketCount(int puid);

    // Save Model Into to Database
    void SaveDatabase();

    // Clear Database
    void ClearDatabase();
};

#endif
