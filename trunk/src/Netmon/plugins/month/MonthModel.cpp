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

int MonthModel::MtModelItem::firstMonth = -1;

MonthModel::MonthModel()
{
    _items[PROCESS_ALL] = MtModelItem();

    InitDatabase();
    ReadDatabase();
}

MonthModel::~MonthModel()
{
    SaveDatabase();
}

void MonthModel::Fill()
{
    Lock();

    // Calculate the desired length of the vectors
    int exMonth = Utils::GetExMonth();
    int length = exMonth - MtModelItem::firstMonth + 1;

    // Fill vectors
    for(std::map<int, MtModelItem>::iterator it = _items.begin(); it != _items.end(); ++it)
    {
        while( (int)it->second.months.size() < length)
        {
            it->second.months.push_back(MonthItem());
        }
    }
    Unlock();
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

    // Set current month as first month when no data is available
    if (MtModelItem::firstMonth == -1 )
    {
        MtModelItem::firstMonth = Utils::GetExMonth();
    }
}

void MonthModel::ReadDatabaseCallback(SQLiteRow *row, void *context)
{
    MonthModel *model = (MonthModel *)context;

    int puid        = row->GetDataInt32(0);
    int date        = row->GetDataInt32(1); // Higher 16 bit for exMonth (Jan 1970 = 0), 
                                            // lower 16 bit for mday
    __int64 txBytes = row->GetDataInt64(2);
    __int64 rxBytes = row->GetDataInt64(3);

    int mDay = Utils::GetMdayByDate(date);
    int exMonth = Utils::GetExMonthByDate(date);

    // Insert an MtViewItem if PUID not Exist
    if (model->_items.count(puid) == 0 )
    {
        model->_items[puid] = MtModelItem();
        MtModelItem::firstMonth = Utils::GetExMonthByDate(date);
    }

    // Fill Vectors
    model->Fill();

    while (exMonth - MtModelItem::firstMonth > (int)model->_items[puid].months.size() - 1)
    {
        model->_items[puid].months.push_back(MonthItem());
    }

    while (exMonth - MtModelItem::firstMonth > (int)model->_items[PROCESS_ALL].months.size() - 1)
    {
        model->_items[PROCESS_ALL].months.push_back(MonthItem());
    }

    // Update Traffic
    MonthItem &mItem = model->_items[puid].months[exMonth - MtModelItem::firstMonth];
    MonthItem &mItemAll = model->_items[PROCESS_ALL].months[exMonth - MtModelItem::firstMonth];

    mItem.dayTx[mDay - 1] = txBytes;
    mItem.dayRx[mDay - 1] = rxBytes;
    mItem.sumTx += txBytes;
    mItem.sumRx += rxBytes;

    mItemAll.dayTx[mDay - 1] += txBytes;
    mItemAll.dayRx[mDay - 1] += rxBytes;
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
    for(it = _items.begin(); it != _items.end(); ++it) // Loop of Process
    {
        int puid = it->first;

        if (puid == PROCESS_ALL )
        {
            continue;
        }

        for(int i = 0; i < (int)it->second.months.size(); i++) // Loop of Month Array
        {
            int exMonth = MtModelItem::firstMonth + i;
            int numDays = Utils::GetNumDays(exMonth);

            for(int j = 0; j < numDays; j++) // Loop of Month
            {
                int date = (exMonth << 16) + (j + 1); // j + 1: [1, 31]

                // Build Command
                TCHAR command[256];

                _stprintf_s(command, _countof(command), 
                    TEXT("Insert Into Traffic Values(%d, %d, %I64d, %I64d, 0, 0);"), 
                    puid, date, it->second.months[i].dayTx[j], it->second.months[i].dayRx[j]);

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
    SQLite::Exec(TEXT("Delete From Traffic;"), true);
}

void MonthModel::InsertPacket(PacketInfoEx *pi)
{
    // Insert an MtViewItem if PUID not Exist
    Lock();
    if (_items.count(pi->puid) == 0 )
    {
        _items[pi->puid] = MtModelItem();
    }
    Unlock();

    // Fill
    Fill();

    // Update Traffic
    int mDay = Utils::GetDay((time_t)pi->time_s);

    Lock();

    MonthItem &mItem = _items[pi->puid].months[Utils::GetExMonth() - MtModelItem::firstMonth];
    MonthItem &mItemAll = _items[PROCESS_ALL].months[Utils::GetExMonth() - MtModelItem::firstMonth];

    if (pi->dir == DIR_UP )
    {
        mItem.dayTx[mDay - 1] += pi->size;
        mItem.sumTx += pi->size;

        mItemAll.dayTx[mDay - 1] += pi->size;
        mItemAll.sumTx += pi->size;
    }
    else if (pi->dir == DIR_DOWN )
    {
        mItem.dayRx[mDay - 1] += pi->size;
        mItem.sumRx += pi->size;

        mItemAll.dayRx[mDay - 1] += pi->size;
        mItemAll.sumRx += pi->size;
    }

    Unlock();
}

void MonthModel::Export(int process, int curMonth, MonthItem &item)
{
    Fill();
    Lock();
    if (_items.count(process) != 0)
    {
        item = _items[process].months[curMonth - MtModelItem::firstMonth];
    }
    Unlock();
}

int MonthModel::GetFirstMonth()
{
    return MtModelItem::firstMonth;
}

int MonthModel::GetLastMonth()
{
    Fill();

    Lock();
    int size = _items[PROCESS_ALL].months.size();
    Unlock();

    return MtModelItem::firstMonth + size - 1;
}
