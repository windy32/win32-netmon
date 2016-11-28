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

#ifndef MONTH_VIEW_H
#define MONTH_VIEW_H

#include "NetView.h"
#include "MonthModel.h"

class MonthView : public NetView
{
protected:
    // Settings
    static int _curMonth;

    // GDI Objects
    static HDC     _hdcTarget;
    static HDC     _hdcBuf;
    static HBITMAP _hbmpBuf;
    static HFONT   _hFontDays;
    static HFONT   _hFontDesc;
    static HPEN    _hPenVertical;

    static HDC     _hdcPage;
    static HBITMAP _hbmpPageUpLight;
    static HBITMAP _hbmpPageUpDark;
    static HBITMAP _hbmpPageDownLight;
    static HBITMAP _hbmpPageDownDark;

    // Model Object
    static MonthModel *_model;

protected:
    static void DrawGraph();
    static void WINAPI TimerProc(HWND hWnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime);

public:
    static void Init(MonthModel *model);
    static void End();
    virtual void SetProcessUid(int puid);

    virtual LRESULT DlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
};

#endif
