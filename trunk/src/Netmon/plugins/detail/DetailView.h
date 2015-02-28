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

#ifndef DETAIL_VIEW_H
#define DETAIL_VIEW_H

#include "../abstract/View.h"
#include "DetailModel.h"

class DetailView : public View
{
private:
    // UI Elements & States
    static HWND _hWnd;
    static HWND _hList;
    static int _iLanguageId;
    static __int64 _curPage;
    static WNDPROC _lpOldProcEdit;

    // Model Object
    static DetailModel *_model;

    // Items in current page
    static std::vector<DetailModel::PacketItem> _items;

private:
    static INT_PTR CALLBACK MyProcEdit(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    static void CALLBACK TimerProc(HWND hWnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime);

    static void UpdateSize(HWND hWnd);
    static void UpdateContent(bool forceRebuild);

    static void ListViewInsert(
        int uid, int puid, int dir, int protocol, int size, __int64 time, int port);

    static void OnPageUp();
    static void OnPageDown();
    static void OnGoto();

    static void SwitchLanguage(HWND hWnd);

public:
    DetailView(DetailModel *model);
    ~DetailView();

public:
    static void SetProcess(int puid);
    static INT_PTR CALLBACK DlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

};

#endif
