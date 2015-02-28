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
#include "DetailView.h"

#include "../../utils/Utils.h"
#include "../../utils/ProcessModel.h"
#include "../../utils//Language.h"

#include "../../res/resource.h"

#pragma region Members of DetailView

// UI Elements & States
HWND DetailView::_hWnd;
HWND DetailView::_hList;
int DetailView::_iLanguageId;
__int64 DetailView::_curPage;
WNDPROC DetailView::_lpOldProcEdit;

// Model Object
DetailModel *DetailView::_model;

// Items in current page
std::vector<DetailModel::PacketItem> DetailView::_items;

#pragma endregion

DetailView::DetailView(DetailModel *model)
{
    _process = PROCESS_ALL;
    _model = model;

    _curPage = 0;
}

DetailView::~DetailView()
{
}

// Get Enter from an edit control
INT_PTR CALLBACK DetailView::MyProcEdit(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    if(uMsg == WM_KEYDOWN && wParam == VK_RETURN )
    {
        OnGoto();
    }
    else
    {
        return CallWindowProc(_lpOldProcEdit, hWnd, uMsg, wParam, lParam);
    }

    return TRUE;
}

void DetailView::SetProcess(int puid)
{
    _process = puid;
    _curPage = 0;

    UpdateContent();
}

void DetailView::TimerProc(HWND hWnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
{
    UpdateContent();
}

// ListView Operations
void DetailView::ListViewInsert(
    int uid, int puid, int dir, int protocol, int size, __int64 time, int port)
{
    // Prepare Columns
    TCHAR szDateTime[MAX_PATH];
    TCHAR szColumn[7][MAX_PATH];

    Language::GetDateTimeString(szDateTime, MAX_PATH, (int)(time >> 32), (int)(time & 0xFFFFFFFF));

    _stprintf_s(szColumn[0], MAX_PATH, TEXT("%d"), uid);
    _stprintf_s(szColumn[1], MAX_PATH, TEXT("%d"), puid);
    _stprintf_s(szColumn[2], MAX_PATH, dir == DIR_UP ? 
        Language::GetString(IDS_DTVIEW_LIST_TX) : 
        Language::GetString(IDS_DTVIEW_LIST_RX));
    _stprintf_s(szColumn[3], MAX_PATH, (protocol == TRA_TCP) ? TEXT("TCP") : 
                                       (protocol == TRA_UDP) ? TEXT("UDP") : 
                                       (protocol == TRA_ICMP) ? TEXT("ICMP") : TEXT("OTHER"));
    _stprintf_s(szColumn[4], MAX_PATH, TEXT("%d"), size);
    _stprintf_s(szColumn[5], MAX_PATH, szDateTime);

    if (protocol == TRA_TCP || protocol == TRA_UDP )
    {
        _stprintf_s(szColumn[6], MAX_PATH, TEXT("%d"), port);
    }
    else
    {
        _stprintf_s(szColumn[6], MAX_PATH, TEXT("-"));
    }

    Utils::ListViewAppend(_hList, 7, 
        szColumn[0], szColumn[1], szColumn[2], szColumn[3], szColumn[4], szColumn[5], szColumn[6]);
}

// Resize & Update Content
void DetailView::UpdateSize(HWND hWnd)
{
    RECT stRect;
    GetClientRect(_hWnd, &stRect);
    int _width  = stRect.right - stRect.left;
    int _height = stRect.bottom - stRect.top;

    MoveWindow(GetDlgItem(hWnd, IDL_DETAIL),   10, 10, _width - 20, _height - 50, TRUE);
    MoveWindow(GetDlgItem(hWnd, IDB_DETAIL_PAGEUP),   10,  _height - 34,  50, 24, TRUE);
    MoveWindow(GetDlgItem(hWnd, IDB_DETAIL_PAGEDOWN), 70,  _height - 34,  50, 24, TRUE);
    MoveWindow(GetDlgItem(hWnd, IDL_DETAIL_STATUS),   130, _height - 30, 450, 20, TRUE);
    MoveWindow(GetDlgItem(hWnd, IDB_DETAIL_GOTO),     _width - 150, _height - 34, 80, 24, TRUE);
    MoveWindow(GetDlgItem(hWnd, IDE_DETAIL_GOTO),     _width - 60,  _height - 34, 50, 24, TRUE);
}

void DetailView::UpdateContent()
{
    // Update Status Label
    TCHAR status[256];
    TCHAR buf[MAX_PATH];
    const TCHAR *szFormat = Language::GetString(IDS_DTVIEW_PAGE); // %s Page %I64d / %I64d - %I64d"

    if (_process == -1)
    {
        _tcscpy_s(buf, MAX_PATH, Language::GetString(IDS_ALL_PROCESS));
    }
    else
    {
        ProcessModel::GetProcessName(_process, buf, MAX_PATH);
    }

    _stprintf_s(status, 256, szFormat, 
        buf, _curPage + 1, _model->GetLastPageIndex(_process) + 1, _model->GetPacketCount(_process));
    SetDlgItemText(_hWnd, IDL_DETAIL_STATUS, status);

    // Update Buttons
    EnableWindow(GetDlgItem(_hWnd, IDB_DETAIL_GOTO), TRUE);
    EnableWindow(GetDlgItem(_hWnd, IDB_DETAIL_PAGEUP), 
        (_curPage + 1 > _model->GetFirstPageIndex(_process) + 1) ? TRUE : FALSE);
    EnableWindow(GetDlgItem(_hWnd, IDB_DETAIL_PAGEDOWN), 
        (_curPage + 1 < _model->GetLastPageIndex(_process) + 1) ? TRUE : FALSE);

    // Update ListView
    if (true) // if need_update()
    {
        std::vector<DetailModel::PacketItem> packets;
        _model->Export(_process, _curPage, packets);

        // item[0]  <---> packets[0]   --+
        // item[1]  <---> packets[1]     | The items are "unchanged"
        // ...            ...            | if corresponding items are the same
        // item[k]  <---> packets[k]   --+
        //                packets[k+1]
        //                ...
        //                packets[n]
        bool unchanged = true;
        if (packets.size() >= _items.size())
        {
            for (unsigned int i = 0; i < _items.size(); i++)
            {
                if (_items[i] != packets[i])
                {
                    unchanged = false;
                }
            }
        }
        else // packet.size() < _items.size()
        {
            unchanged = false;
        }

        if (unchanged)
        {
            for (unsigned int i = _items.size(); i < packets.size(); i++)
            {
                ListViewInsert(
                    packets[i].uid, 
                    packets[i].puid, 
                    packets[i].dir,
                    packets[i].protocol,
                    packets[i].size,
                    packets[i].time,
                    packets[i].port);
            }
        }
        else
        {
            Utils::ListViewClear(_hList);

            for (unsigned int i = 0; i < packets.size(); i++)
            {
                ListViewInsert(
                    packets[i].uid, 
                    packets[i].puid, 
                    packets[i].dir,
                    packets[i].protocol,
                    packets[i].size,
                    packets[i].time,
                    packets[i].port);
            }
        }

        _items = packets;
    }
}

// Flip Page
void DetailView::OnPageUp()
{
    _curPage -= 1;
    UpdateContent();

    EnableWindow(GetDlgItem(_hWnd, IDB_DETAIL_PAGEUP), 
        (_curPage + 1 > _model->GetFirstPageIndex(_process) + 1) ? TRUE : FALSE);
    EnableWindow(GetDlgItem(_hWnd, IDB_DETAIL_PAGEDOWN), 
        (_curPage + 1 < _model->GetLastPageIndex(_process) + 1) ? TRUE : FALSE);
}

void DetailView::OnPageDown()
{
    _curPage += 1;
    UpdateContent();

    EnableWindow(GetDlgItem(_hWnd, IDB_DETAIL_PAGEUP), 
        (_curPage + 1 > _model->GetFirstPageIndex(_process) + 1) ? TRUE : FALSE);
    EnableWindow(GetDlgItem(_hWnd, IDB_DETAIL_PAGEDOWN), 
        (_curPage + 1 < _model->GetLastPageIndex(_process) + 1) ? TRUE : FALSE);
}

void DetailView::OnGoto()
{
    BOOL translated;
    int page = GetDlgItemInt(_hWnd, IDE_DETAIL_GOTO, &translated, TRUE);

    if (!translated ) 
    {
        MessageBox(_hWnd, Language::GetString(IDS_DTVIEW_NUM_INCORRECT),
            TEXT("Error"), MB_ICONWARNING | MB_OK);
    }
    else if (page <= 0 || page > _model->GetLastPageIndex(_process) + 1 )
    {
        MessageBox(_hWnd, Language::GetString(IDS_DTVIEW_NUM_OUT_OF_RANGE),
            TEXT("Error"), MB_ICONWARNING | MB_OK);
    }
    else
    {
        _curPage = page - 1;
        UpdateContent();

        EnableWindow(GetDlgItem(_hWnd, IDB_DETAIL_PAGEUP), 
            (_curPage + 1 > _model->GetFirstPageIndex(_process) + 1) ? TRUE : FALSE);
        EnableWindow(GetDlgItem(_hWnd, IDB_DETAIL_PAGEDOWN), 
            (_curPage + 1 < _model->GetLastPageIndex(_process) + 1) ? TRUE : FALSE);

        SetDlgItemText(_hWnd, IDE_DETAIL_GOTO, TEXT(""));
    }
}

void DetailView::SwitchLanguage(HWND hWnd)
{
    // List view column header
    Utils::ListViewSetColumnText(GetDlgItem(hWnd, IDL_DETAIL), 7, 
        Language::GetString(IDS_DTVIEW_LIST_UID),
        Language::GetString(IDS_DTVIEW_LIST_PROCESS),
        Language::GetString(IDS_DTVIEW_LIST_DIR),
        Language::GetString(IDS_DTVIEW_LIST_PROTOCOL),
        Language::GetString(IDS_DTVIEW_LIST_SIZE),
        Language::GetString(IDS_DTVIEW_LIST_TIME),
        Language::GetString(IDS_DTVIEW_LIST_PORT));

    // List view content & label
    UpdateContent();

    // Button
    SetDlgItemText(hWnd, IDB_DETAIL_GOTO, Language::GetString(IDS_DTVIEW_GOTO));
}

// Dialog Procedure
INT_PTR DetailView::DlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    if (uMsg == WM_INITDIALOG )
    {
        _hWnd = hWnd;
        _hList = GetDlgItem(hWnd, IDL_DETAIL);

        _lpOldProcEdit = 
            (WNDPROC)SetWindowLong(GetDlgItem(_hWnd, IDE_DETAIL_GOTO), GWL_WNDPROC, (LONG)MyProcEdit);

        // Save current language id
        _iLanguageId = Language::GetLangId();

        // Init button
        SetDlgItemText(hWnd, IDB_DETAIL_GOTO, Language::GetString(IDS_DTVIEW_GOTO));

        // Init ListView
        Utils::ListViewInit(_hList, FALSE, 7, 
            Language::GetString(IDS_DTVIEW_LIST_UID),
            Language::GetString(IDS_DTVIEW_LIST_PROCESS),
            Language::GetString(IDS_DTVIEW_LIST_DIR),
            Language::GetString(IDS_DTVIEW_LIST_PROTOCOL),
            Language::GetString(IDS_DTVIEW_LIST_SIZE),
            Language::GetString(IDS_DTVIEW_LIST_TIME),
            Language::GetString(IDS_DTVIEW_LIST_PORT),
            60, 70, 50, 80, 60, 220, 60);

        // Clear items
        _items.clear();

        UpdateContent();

        // Start Timer
        SetTimer(hWnd, 0, 1000, DetailView::TimerProc);
    }
    else if (uMsg == WM_PAINT )
    {
        if (Language::GetLangId() != _iLanguageId )
        {
            SwitchLanguage(hWnd);
            _iLanguageId = Language::GetLangId();
        }
        return FALSE;
    }
    else if (uMsg == WM_COMMAND )
    {
        if (wParam == IDB_DETAIL_PAGEUP )
        {
            OnPageUp();
        }
        else if (wParam == IDB_DETAIL_PAGEDOWN )
        {
            OnPageDown();
        }
        else if (wParam == IDB_DETAIL_GOTO )
        {
            OnGoto();
        }
    }
    else if(uMsg == WM_CTLCOLORSTATIC || uMsg == WM_CTLCOLORBTN )
    {
        return RGB(255, 255, 255);
    }
    else if(uMsg == WM_CTLCOLORDLG )
    {
        return (INT_PTR)GetStockObject(WHITE_BRUSH);
    }
    else if (uMsg == WM_CLOSE )
    {
        KillTimer(hWnd, 0);
        DestroyWindow(hWnd);
    }
    else if (uMsg == WM_SIZE )
    {
        UpdateSize(hWnd);
    }
    else
    {
        return FALSE;
    }

    return TRUE;
}
