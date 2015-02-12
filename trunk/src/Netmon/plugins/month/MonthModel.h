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

#ifndef MONTH_MODEL_H
#define MONTH_MODEL_H

#include "../abstract/Model.h"
#include "../../Utils/SQLite.h"

class MonthModel : public Model
{
public:
    // Item Definition
    // 1. The Item of a Process for a Month, 496 Bytes -------
    typedef struct tagMonthItem
    {
        __int64 dayTx[31]; // In Bytes
        __int64 dayRx[31];

        __int64 sumTx;
        __int64 sumRx;

        struct tagMonthItem()
        {
            RtlZeroMemory(dayTx, sizeof(dayTx));
            RtlZeroMemory(dayRx, sizeof(dayRx));

            sumTx = 0;
            sumRx = 0;
        }
    } MonthItem;

private:
    // 2. The Item of a Process ------------------------------
    typedef struct tagMtModelItem
    {
        static int firstMonth; // Jan 1970 = 0, Feb 1970 = 1 ...
        std::vector<MonthItem> months;
    } MtModelItem;

    // Items
    std::map<int, MtModelItem> _items;

    // Others
    static MonthModel *_this;

private:
    void Fill();
    void InitDatabase();
    static void InitDatabaseCallback(SQLiteRow *row);

public:
    MonthModel();

    // Modify the Model
    void InsertPacket(PacketInfoEx *pi);

    // Export Model Info
    void Export(int process, int curMonth, MonthItem &item);

    int GetFirstMonth();
    int GetLastMonth();

    // Save Model Info to Database
    void SaveDatabase();
};

#endif
