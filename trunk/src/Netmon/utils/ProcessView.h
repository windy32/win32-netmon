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

#ifndef PROCESS_VIEW_H
#define PROCESS_VIEW_H

#include "SQLite.h"
#include "Packet.h"
#include "ProcessModel.h"

class ProcessView
{
private:
    // It's associated with a ListView control
    static HWND _hList;
    static bool _hideProcess;
    static bool _prevHideProcess;

private: // Utils Used by ProcessView
    static void ListViewInsert(const ProcessModel::ProcessItem &item);
    static void ListViewUpdate(int index, const ProcessModel::ProcessItem &item);

public:
    static void Init(HWND hList);
    static void Update(bool init = false, bool redraw = false);
    static void HideProcesses();
    static void ShowProcesses();
    static bool IsHidden();
};

#endif