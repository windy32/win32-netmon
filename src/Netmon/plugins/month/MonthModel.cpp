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
#include "MonthModel.h"
#include "../../Utils/Utils.h"
#include "../../Utils/ProcessModel.h"

MonthModel::MonthModel()
{
    _items[PROCESS_ALL] = MtModelItem();
    _clear_flag = false;

    InitDatabase();
    ReadDatabase();
}

MonthModel::~MonthModel()
{
    if (_clear_flag)
    {
        SQLite::Exec(TEXT("Delete From Traffic;"), true);
        SQLite::Flush();
        SQLite::Exec(TEXT("Vacuum;"), false);
    }
    else
    {
        SaveDatabase();
    }
}

void MonthModel::InitDatabase()
{
    if (!SQLite::TableExist(TEXT("Traffic")))
    {
        SQLite::Exec(TEXT("Create Table Traffic(")
                     TEXT("    ProcessUID     Integer,")
                     TEXT("    Date           Integer,")
                     TEXT("    TxBytes        Integer,")
                     TEXT("    RxBytes        Integer,")
                     TEXT("    TxPackets      Integer,")
                     TEXT("    RxPackets      Integer,")
                     TEXT("    ")
                     TEXT("    Primary Key (ProcessUID, Date),")
                     TEXT("    Foreign Key (ProcessUID) References Process(UID)")
                     TEXT(");"), true);
    }
}

void MonthModel::ReadDatabase()
{
    TCHAR command[256];

    // Build Command
    _stprintf_s(command, _countof(command), TEXT("Select * From Traffic;"));

    // Build SQLiteRow Object
    SQLiteRow row;

    row.InsertType(SQLiteRow::TYPE_INT32); // 0 ProcessUid
    row.InsertType(SQLiteRow::TYPE_INT32); // 1 Date
    row.InsertType(SQLiteRow::TYPE_INT64); // 2 TxBytes
    row.InsertType(SQLiteRow::TYPE_INT64); // 3 RxBytes
    row.InsertType(SQLiteRow::TYPE_INT32); // 4 TxPackets
    row.InsertType(SQLiteRow::TYPE_INT32); // 5 RxPackets

    // Select
    SQLite::Select(command, &row, ReadDatabaseCallback, this);
}

void MonthModel::ReadDatabaseCallback(SQLiteRow *row, void *context)
{
    MonthModel *model = (MonthModel *)context;

    int puid        = row->GetDataInt32(0);
    int time        = row->GetDataInt32(1); // 32 bit time_t
    __int64 txBytes = row->GetDataInt64(2);
    __int64 rxBytes = row->GetDataInt64(3);

    Date date(time);
    ShortDate sdate = date.ToShortDate();

    // Insert an MtViewItem if PUID not Exist
    if (model->_items.count(puid) == 0)
    {
        model->_items[puid] = MtModelItem();
    }

    // Insert an MonthItem if Date not Exist
    if (model->_items[puid].count(sdate) == 0)
    {
        model->_items[puid][sdate] = MonthItem();
    }
    if (model->_items[PROCESS_ALL].count(sdate) == 0)
    {
        model->_items[PROCESS_ALL][sdate] = MonthItem();
    }

    // Update Traffic
    MonthItem &mItem = model->_items[puid][sdate];
    MonthItem &mItemAll = model->_items[PROCESS_ALL][sdate];

    mItem.dayTx[date.mday - 1] = txBytes;
    mItem.dayRx[date.mday - 1] = rxBytes;
    mItem.sumTx += txBytes;
    mItem.sumRx += rxBytes;

    mItemAll.dayTx[date.mday - 1] += txBytes;
    mItemAll.dayRx[date.mday - 1] += rxBytes;
    mItemAll.sumTx += txBytes;
    mItemAll.sumRx += rxBytes;
}

void MonthModel::SaveDatabase()
{
    Lock();

    // Delete all records
    SQLite::Exec(TEXT("Delete From Traffic;"), true);

    // Insert records
    std::map<int, MtModelItem>::iterator it;
    for (it = _items.begin(); it != _items.end(); ++it) // Loop of Process
    {
        int puid = it->first;
        if (puid == PROCESS_ALL)
        {
            continue;
        }

        std::map<ShortDate, MonthItem>::iterator i;
        for (i = it->second.begin(); i != it->second.end(); ++i) // Loop of Pages
        {
            ShortDate sdate = i->first;
            MonthItem item = i->second;
            int totalDays = Date::GetTotalDays(sdate.year, sdate.month);

            for (int j = 0; j < totalDays; j++) // Loop of Days
            {
                Date date(sdate.year, sdate.month, j + 1);

                if (sdate.month == -1)
                {
                    int stopHere = 0;
                }

                // Build Command
                TCHAR command[256];
                _stprintf_s(command, _countof(command), 
                    TEXT("Insert Into Traffic Values(%d, %d, %I64d, %I64d, 0, 0);"), 
                    puid, date.ToInt32(), item.dayTx[j], item.dayRx[j]);

                // Insert
                SQLite::Exec(command, true);
            }
        }
    }

    Unlock();

    // Flush
    SQLite::Flush();
}

void MonthModel::ClearDatabase()
{
    _clear_flag = true;
}

void MonthModel::InsertPacket(PacketInfoEx *pi)
{
    Lock();

    // Create date object
    Date date(pi->time_s);
    ShortDate sdate(date.year, date.month);

    // Insert an MtViewItem if PUID not Exist
    if (_items.count(pi->puid) == 0)
    {
        _items[pi->puid] = MtModelItem();
    }

    // Insert an MonthItem if Date not Exist
    if (_items[pi->puid].count(sdate) == 0)
    {
        _items[pi->puid][sdate] = MonthItem();
    }
    if (_items[PROCESS_ALL].count(sdate) == 0)
    {
        _items[PROCESS_ALL][sdate] = MonthItem();
    }

    MonthItem &mItem = _items[pi->puid][sdate];
    MonthItem &mItemAll = _items[PROCESS_ALL][sdate];

    if (pi->dir == DIR_UP)
    {
        mItem.dayTx[date.mday - 1] += pi->size;
        mItem.sumTx += pi->size;

        mItemAll.dayTx[date.mday - 1] += pi->size;
        mItemAll.sumTx += pi->size;
    }
    else if (pi->dir == DIR_DOWN)
    {
        mItem.dayRx[date.mday - 1] += pi->size;
        mItem.sumRx += pi->size;

        mItemAll.dayRx[date.mday - 1] += pi->size;
        mItemAll.sumRx += pi->size;
    }

    Unlock();
}

void MonthModel::Export(int process, const ShortDate &sdate, MonthItem &item)
{
    Lock();
    if (_items.count(process) != 0)
    {
        item = _items[process][sdate];
    }
    Unlock();
}

ShortDate MonthModel::GetFirstMonth(int puid)
{
    ShortDate sdate = ShortDate::Null; // Default: nothing exist (which is unlikely to happen)

    Lock();
    std::map<ShortDate, MonthItem> &item = _items[puid];
    std::map<ShortDate, MonthItem>::iterator it = item.begin();

    if (it != item.end()) // There is at least one key-value pair
    {
        sdate = it->first;
    }
    Unlock();

    return sdate;
}

ShortDate MonthModel::GetLastMonth(int puid)
{
    ShortDate sdate = ShortDate::Null; // Default: nothing exist (which is unlikely to happen)

    Lock();
    std::map<ShortDate, MonthItem> &item = _items[puid];
    std::map<ShortDate, MonthItem>::reverse_iterator it = item.rbegin();

    if (it != item.rend()) // There is at least one key-value pair
    {
        sdate = it->first;
    }
    Unlock();

    return sdate;
}

ShortDate MonthModel::GetClosestMonth(int puid, const ShortDate &target)
{
    ShortDate sdate; // Default: nothing exist (which is unlikely to happen)

    Lock();
    std::map<ShortDate, MonthItem> &item = _items[puid];
    std::map<ShortDate, MonthItem>::iterator it = item.begin();

    if (it == item.end()) // There's no month items
    {
        sdate = ShortDate::Null;
    }
    else // There are some items, pick the one which is closest to target
    {
        int minDiff = INT_MAX;
        ShortDate minDate;

        for (it = item.begin(); it != item.end(); ++it)
        {
            ShortDate month = it->first;
            int diff = target.DiffFrom(month);

            if (diff < minDiff)
            {
                minDiff = diff;
                minDate = month;
            }
        }
        
        sdate = minDate;
    }
    Unlock();

    return sdate;
}
