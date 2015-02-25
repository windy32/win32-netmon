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
#include "MonthView.h"

#include "../../utils/Utils.h"
#include "../../utils/ProcessModel.h"

#include "../../res/resource.h"

#pragma region Members of MonthView

// Settings
ShortDate MonthView::_curMonth;

// GDI Objects
HFONT   MonthView::_hFontDays;
HFONT   MonthView::_hFontDesc;
HPEN    MonthView::_hPenVertical;

HDC     MonthView::_hdcTarget;
HDC     MonthView::_hdcBuf;
HBITMAP MonthView::_hbmpBuf;

HDC     MonthView::_hdcPage;
HBITMAP MonthView::_hbmpPageUpLight;
HBITMAP MonthView::_hbmpPageUpDark;
HBITMAP MonthView::_hbmpPageDownLight;
HBITMAP MonthView::_hbmpPageDownDark;

// Window Handle
HWND    MonthView::_hWnd;

// Model Object
MonthModel *MonthView::_model;

#pragma endregion

MonthView::MonthView(MonthModel *model)
{
    _process = PROCESS_ALL;
    _model = model;

    _hdcBuf = 0;  // the device context
    _hbmpBuf = 0; // and the bitmap are not deleted when user goes to another tab

    _curMonth = model->GetLastMonth(PROCESS_ALL);

    // No packet has been captured for any process
    if (_curMonth == ShortDate::Null)
    {
        Date date(time(0));
        _curMonth = ShortDate(date.year, date.month);
    }
}

MonthView::~MonthView()
{
    DeleteDC(_hdcBuf);
    DeleteObject(_hbmpBuf);

    DeleteDC(_hdcPage);
    DeleteObject(_hbmpPageUpLight);
    DeleteObject(_hbmpPageUpDark);
    DeleteObject(_hbmpPageDownLight);
    DeleteObject(_hbmpPageDownDark);
}

void MonthView::SetProcess(int puid)
{
    _process = puid;

    if (_curMonth > _model->GetLastMonth(puid))
    {
        _curMonth = _model->GetLastMonth(puid);
    }

    DrawGraph();
}

void MonthView::TimerProc(HWND hWnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
{
    DrawGraph();
}

void MonthView::DrawGraph()
{
    RECT stRect;
    GetClientRect(_hWnd, &stRect);

    int _width  = stRect.right - stRect.left;
    int _height = stRect.bottom - stRect.top;

    // Export Model Info
    MonthModel::MonthItem item;

    // If date not exist, item is not updated, all fields are 0 bytes
    _model->Export(_process, _curMonth, item);

    int numDays = Date::GetTotalDays(_curMonth.year, _curMonth.month);

    // Rectangle for Graph
    int x1 = 8;
    int y1 = 19;
    int x2 = _width - 48;
    int y2 = _height - 26;

    int colWidth = (_width - 8 - 48) / numDays;
    colWidth = min(22, colWidth);

    x2 = x1 + colWidth * numDays;

    // Calculate the Maximum Amount of Traffic in a Day
    __int64 maxTraffic = 0;

    for(int i = 0; i < numDays; i++)
    {
        if (item.dayTx[i] > maxTraffic)
        {
            maxTraffic = item.dayTx[i];
        }

        if (item.dayRx[i] > maxTraffic)
        {
            maxTraffic = item.dayRx[i];
        }
    }

    maxTraffic /= (1024 * 1024); // Unit is now MB

    // Decide Scale
    //    1.  (    0 MB,    10 MB]    10,     5,   2.5,  1.25
    //    2.  (   10 MB,    20 MB]    20,    10,     5,   2.5
    //    3.  (   20 MB,    50 MB]    50,    25,  12.5,  6.25
    //    4.  (   50 MB,   100 MB]   100,    50,    25,  12.5
    //    5.  (  100 MB,   200 MB]   200,   100,    50,    25
    //    6.  (  200 MB,   500 MB]   500,   250,   125,  62.5
    //    7.  (  500 MB,  1000 MB]  1000,   500,   250,   125
    //    8.  ( 1000 MB,  2000 MB]  2000,  1000,   500,   250
    //    9.  ( 2000 MB,  5000 MB]  5000,  2500,  1250,   625
    //    10. ( 5000 MB, 10000 MB] 10000,  5000,  2500,  1250
    //    11. (10000 MB, 20000 MB] 20000, 10000,  5000,  2500
    //    12. (20000 MB, 50000 MB] 50000, 25000, 12500,  6250
    //    ( ...More traffic are not currently supported )
    const int traffic[12] = {10, 20, 50, 100, 200, 500, 1000, 2000, 5000, 10000, 20000, 50000};
    const int precision[12][4] = 
    {
        {0, 0, 1, 2}, {0, 0, 0, 1}, {0, 0, 1, 2}, 
        {0, 0, 0, 1}, {0, 0, 0, 0}, {0, 0, 0, 1}, 
        {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, 
        {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}
    };
    int scaleTraffic = 0;
    int scalePrecision[4];

    for(int i = 0; i < 12; i++)
    {
        if (maxTraffic < traffic[i] )
        {
            scaleTraffic = traffic[i];
            scalePrecision[0] = precision[i][0];
            scalePrecision[1] = precision[i][1];
            scalePrecision[2] = precision[i][2];
            scalePrecision[3] = precision[i][3];
            break;
        }
    }

    if (scaleTraffic == 0 ) // More than 50000 MB
    {
        scaleTraffic = traffic[12 - 1];
        scalePrecision[0] = precision[12 - 1][0];
        scalePrecision[1] = precision[12 - 1][1];
        scalePrecision[2] = precision[12 - 1][2];
        scalePrecision[3] = precision[12 - 1][3];
    }

    // Clear Background
    Rectangle(_hdcBuf, -1, -1, _width + 1, _height + 1);

    // Bg for Days
    SelectObject(_hdcBuf, GetStockObject(NULL_PEN));
    SetDCBrushColor(_hdcBuf, RGB(0xF0, 0xF0, 0xF0));

    Rectangle(_hdcBuf, x1, y1 - 12, x2, y1 + 2);

    // Box for Graph
    SelectObject(_hdcBuf, GetStockObject(DC_PEN));
    SelectObject(_hdcBuf, GetStockObject(NULL_BRUSH));

    SetDCPenColor(_hdcBuf, RGB(0x80, 0x80, 0x80));
    Rectangle(_hdcBuf, x1, y1 - 12, x2, y2 + 1);

    SetDCPenColor(_hdcBuf, RGB(0xBB, 0xBB, 0xBB));
    Rectangle(_hdcBuf, x1, y1 - 12, x2, y2);

    SelectObject(_hdcBuf, GetStockObject(DC_BRUSH));

    // Draw Vertical Lines
    SelectObject(_hdcBuf, _hPenVertical);

    for(int i = 1; i < numDays; i++)
    {
        MoveToEx(_hdcBuf, x1 + i * colWidth, y1 + 1 - 12, 0);
        LineTo(_hdcBuf, x1 + i * colWidth, y2 - 1);
    }

    // Draw Horizontal Lines
    SelectObject(_hdcBuf, GetStockObject(DC_PEN));
    SetDCPenColor(_hdcBuf, RGB(0xBB, 0xBB, 0xBB));

    for(int i = 1; i < 4; i++)
    {
        MoveToEx(_hdcBuf, x1 + 1, y1 + (y2 - y1) * i / 4, 0);
        LineTo(_hdcBuf, x2 - 1, y1 + (y2 - y1) * i / 4);
    }

    SetDCPenColor(_hdcBuf, RGB(0x00, 0x00, 0x00));

    // Draw Days
    SelectObject(_hdcBuf, _hFontDays);
    SetTextAlign(_hdcBuf, TA_CENTER);

    for (int i = 0; i < numDays; i++)
    {
        TCHAR szDay[32];
        _stprintf_s(szDay, _countof(szDay), TEXT("%d"), i + 1);

        // Set Color, Sundays are Red
        if (Date(_curMonth.year, _curMonth.month, i + 1).wday == 0)
        {
            SetTextColor(_hdcBuf, RGB(0xDF, 0x00, 0x24));
        }
        else
        {
            SetTextColor(_hdcBuf, RGB(0x77, 0x77, 0x77));
        }

        TextOut(_hdcBuf, x1 + colWidth / 2 + i * colWidth, y1 + 2 - 12, szDay, _tcslen(szDay));
    }

    // Draw Scale
    TCHAR yAxisText[16];
    TCHAR yAxisTextFormat[16];
    SetTextAlign(_hdcBuf, TA_LEFT);
    SetTextColor(_hdcBuf, RGB(0x00, 0x00, 0x00));

    for (int i = 0; i < 4; i++)
    {
        _stprintf_s(yAxisTextFormat, _countof(yAxisTextFormat), 
            TEXT("%%.%dlf MB"), scalePrecision[i]);
        _stprintf_s(yAxisText, _countof(yAxisText), 
            yAxisTextFormat, scaleTraffic / (double)(1 << i));
        TextOut(_hdcBuf, x2 + 3, y1 + (y2 - y1) * i / 4 - 5 , yAxisText, _tcslen(yAxisText));
    }

    // Draw Traffic

    // - Rx
    SetDCBrushColor(_hdcBuf, RGB(0x22, 0x8B, 0x22));

    for (int i = 0; i < numDays; i++)
    {
        double rxTraffic = (item.dayRx[i] >> 10) / (scaleTraffic * 1024.0);
        int yPos = 
            (rxTraffic < 0.125) ? 
            (int)((y2 - y1) * rxTraffic * 2.0) : // Linear
            (int)((y2 - y1) * (1.0 + Utils::Log(rxTraffic, 2.0) / 4.0)); // Log
        Rectangle(_hdcBuf, 
            x1 + 4 + colWidth * i, y2 - yPos - 1, 
            x1 + 4 + colWidth * i + (colWidth - 8) / 2, y2);
    }

    // - Tx
    SetDCBrushColor(_hdcBuf, RGB(0xDF, 0x00, 0x24));

    for(int i = 0; i < numDays; i++)
    {
        double txTraffic = (item.dayTx[i] >> 10) / (scaleTraffic * 1024.0);
        int yPos = 
            (txTraffic < 0.125) ? 
            (int)((y2 - y1) * txTraffic * 2.0) : // Linear
            (int)((y2 - y1) * (1.0 + Utils::Log(txTraffic, 2.0) / 4.0)); // Log
        Rectangle(_hdcBuf, 
            x1 + 4 + colWidth * i + (colWidth - 8) / 2, y2 - yPos - 1, 
            x1 + colWidth - 4 + colWidth * i, y2);
    }

    SetDCBrushColor(_hdcBuf, RGB(0xFF, 0xFF, 0xfF));

    // Process Name & Statistics
    SelectObject(_hdcBuf, _hFontDesc);
    SetTextAlign(_hdcBuf, TA_LEFT);
    SetTextColor(_hdcBuf, RGB(0x00, 0x00, 0x00));

    TCHAR szText[256];
    TCHAR szYearMonth[256];
    Language::GetYearMonthString(szYearMonth, 256, _curMonth.year + 1900, _curMonth.month);

    TCHAR processName[MAX_PATH];
    if (_process == -1)
    {
        _tcscpy_s(processName, MAX_PATH, Language::GetString(IDS_ALL_PROCESS));
    }
    else
    {
        ProcessModel::GetProcessName(_process, processName, MAX_PATH);
    }

    if (item.sumRx < 1024 * 1024 && item.sumTx < 1024 * 1024) // KB_KB
    {
        // Like "%s - %s (Incoming: %d KB / Outgoing: %d KB)"
        const TCHAR *szFormat = Language::GetString(IDS_MTVIEW_TEXT_KB_KB);
        _stprintf_s(szText, _countof(szText), szFormat, 
            processName, szYearMonth, (int)(item.sumRx >> 10), (int)(item.sumTx >> 10));
    }
    else if (item.sumRx < 1024 * 1024 && item.sumTx >= 1024 * 1024) // KB_MB
    {
        const TCHAR *szFormat = Language::GetString(IDS_MTVIEW_TEXT_KB_MB);
        _stprintf_s(szText, _countof(szText), szFormat, 
            processName, szYearMonth, (int)(item.sumRx >> 10), (int)(item.sumTx >> 20));
    }
    else if (item.sumRx >= 1024 * 1024 && item.sumTx < 1024 * 1024) // MB_KB
    {
        const TCHAR *szFormat = Language::GetString(IDS_MTVIEW_TEXT_MB_KB);
        _stprintf_s(szText, _countof(szText), szFormat, 
            processName, szYearMonth, (int)(item.sumRx >> 20), (int)(item.sumTx >> 10));
    }
    else // MB_MB
    {
        const TCHAR *szFormat = Language::GetString(IDS_MTVIEW_TEXT_MB_MB);
        _stprintf_s(szText, _countof(szText), szFormat, 
            processName, szYearMonth, (int)(item.sumRx >> 20), (int)(item.sumTx >> 20));
    }

    TextOut(_hdcBuf, x1 + 1, y2 + 2, szText, _tcslen(szText));

    // Draw PageUp / PageDown Icon
    if (_curMonth == _model->GetFirstMonth(_process)) // No previous month
    {
        SelectObject(_hdcPage, _hbmpPageUpLight);
    }
    else
    {
        SelectObject(_hdcPage, _hbmpPageUpDark);
    }

    BitBlt(_hdcBuf, x2 - 21, y2 + 4, 7, 7, _hdcPage, 0, 0, SRCCOPY);

    if (_curMonth == _model->GetLastMonth(_process)) // No next month
    {
        SelectObject(_hdcPage, _hbmpPageDownLight);
    }
    else
    {
        SelectObject(_hdcPage, _hbmpPageDownDark);
    }

    BitBlt(_hdcBuf, x2 - 9, y2 + 4, 7, 7, _hdcPage, 0, 0, SRCCOPY);

    // Write to Screen
    BitBlt(_hdcTarget, 0, 0, _width, _height, _hdcBuf, 0, 0, SRCCOPY);
}

INT_PTR MonthView::DlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    if (uMsg == WM_INITDIALOG )
    {
        _hWnd = hWnd;

        // Init GDI Objects

        // - Device Context & Bitmap
        _hdcTarget = GetDC(hWnd);

        if (_hdcBuf == 0 )
        {
            _hdcBuf = CreateCompatibleDC(_hdcTarget);
            _hbmpBuf = CreateCompatibleBitmap(_hdcTarget, 2000, 1200);  // Suppose enough

            SelectObject(_hdcBuf, _hbmpBuf);

            _hdcPage = CreateCompatibleDC(_hdcTarget);

            _hbmpPageUpLight   = LoadBitmap(GetModuleHandle(0), MAKEINTRESOURCE(IDB_PAGEUP_LT));
            _hbmpPageUpDark    = LoadBitmap(GetModuleHandle(0), MAKEINTRESOURCE(IDB_PAGEUP_DK));
            _hbmpPageDownLight = LoadBitmap(GetModuleHandle(0), MAKEINTRESOURCE(IDB_PAGEDN_LT));
            _hbmpPageDownDark  = LoadBitmap(GetModuleHandle(0), MAKEINTRESOURCE(IDB_PAGEDN_DK));
        }

        // - Font
        _hFontDays = Utils::MyCreateFont(TEXT("Arial"), 12, 5, false);
        _hFontDesc = Utils::MyCreateFont(TEXT("MS Shell Dlg 2"), 14, 0, true);

        // - Pen
        _hPenVertical = CreatePen(PS_DOT, 0, RGB(0xC4, 0xC4, 0xC4));

        SelectObject(_hdcBuf, GetStockObject(DC_PEN));

        // - Brush
        SelectObject(_hdcBuf, GetStockObject(DC_BRUSH));

        // - Background Mode
        SetBkMode(_hdcBuf, TRANSPARENT);

        // Start Timer
        SetTimer(hWnd, 0, 1000, MonthView::TimerProc);
    }
    else if (uMsg == WM_CLOSE )
    {
        KillTimer(hWnd, 0);

        ReleaseDC(hWnd, _hdcTarget);
        _hdcTarget = 0;

        DestroyWindow(hWnd);
    }
    else if (uMsg == WM_LBUTTONDOWN )
    {
        // Get Width and Height
        RECT stRect;
        GetClientRect(_hWnd, &stRect);

        int _width  = stRect.right - stRect.left;
        int _height = stRect.bottom - stRect.top;

        // Rectangle for Graph
        int numDays = Date::GetTotalDays(_curMonth.year, _curMonth.month);

        int x1 = 8;
        int y1 = 19;
        int x2 = _width - 48;
        int y2 = _height - 26;

        int colWidth = (_width - 8 - 48) / numDays;
        colWidth = min(22, colWidth);

        x2 = x1 + colWidth * numDays;

        // Page State
        int xPos = GET_X_LPARAM(lParam); 
        int yPos = GET_Y_LPARAM(lParam);

        if (xPos >= x2 - 21 && xPos <= x2 - 21 + 7 &&
            yPos >= y2 + 4  && yPos <= y2 + 4  + 7 )
        {
            if (_curMonth > _model->GetFirstMonth(_process)) // Previous month
            {
                _curMonth = _curMonth.PrevMonth();
                DrawGraph();
            }
        }
        else if (xPos >= x2 - 9 && xPos <= x2 - 9 + 7 &&
                 yPos >= y2 + 4 && yPos <= y2 + 4 + 7 )
        {
            if (_curMonth < _model->GetLastMonth(_process)) // Next month
            {
                _curMonth = _curMonth.NextMonth();
                DrawGraph();
            }
        }
    }
    else if (uMsg == WM_PAINT )
    {
        PAINTSTRUCT stPS;
        BeginPaint(hWnd, &stPS);

        DrawGraph();

        EndPaint(hWnd, &stPS);
    }
    else if (uMsg == WM_SIZE )
    {
        DrawGraph();
    }
    else
    {
        return FALSE;
    }
    
    return TRUE;
}
