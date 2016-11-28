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

#ifndef NET_VIEW_H
#define NET_VIEW_H

#include "../utils/Packet.h"
#include "../utils/SQLite.h"
#include "../utils/Language.h"

// Base class of all concrete views
class NetView
{
protected:
    static int      _process;
    static const int PROCESS_ALL;

    static int     _width;
    static int     _height;

public:
    virtual void SetProcessUid(int puid) = 0;
    virtual LRESULT DlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) = 0;
};

#endif
