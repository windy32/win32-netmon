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
#include "RealtimeView.h"

#include "../../Utils/Utils.h"
#include "../../Utils/ProcessModel.h"

#pragma region Members of RealtimeView

// Settings
enum ZoomFactor RealtimeView::_zoomFactor;
enum RealtimeView::SmoothFactor RealtimeView::_smoothFactor;

// GDI Objects
HDC     RealtimeView::_hdcTarget;
HDC     RealtimeView::_hdcBuf;
HBITMAP RealtimeView::_hbmpBuf;

HFONT   RealtimeView::_hEnglishFont;
HFONT   RealtimeView::_hShellDlgFont;
HFONT   RealtimeView::_hProcessFont;

// Window Handle
HWND    RealtimeView::_hWnd;

// Model Object
RealtimeModel *RealtimeView::_model;

#pragma endregion

RealtimeView::RealtimeView(RealtimeModel *model)
{
    _process = PROCESS_ALL;
    _model = model;

    _hdcBuf = 0;  // the device context
    _hbmpBuf = 0; // and the bitmap are not deleted when user goes to another tab

    _zoomFactor = ZOOM_1S;
    _smoothFactor = SMOOTH_1X;
}

RealtimeView::~RealtimeView()
{
    DeleteDC(_hdcBuf);
    DeleteObject(_hbmpBuf);
}

void RealtimeView::SetProcess(int puid)
{
    _process = puid;
    DrawGraph();
}

void RealtimeView::TimerProc(HWND hWnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
{
    DrawGraph();
}

void RealtimeView::DrawGraph()
{
    RECT stRect;
    GetClientRect(_hWnd, &stRect);

    int _width  = stRect.right - stRect.left;
    int _height = stRect.bottom - stRect.top;

    // - Rectangle for Graph
    int x1 = 34;
    int y1 = 10;
    int x2 = _width - 10;
    int y2 = _height - 26;

    if (x2 % 2 == 1)
    {
        x2 -= 1;
    }

    // Clear Background
    Rectangle(_hdcBuf, -1, -1, _width + 1, _height + 1);

    // Box for Graph
    Rectangle(_hdcBuf, x1, y1, x2, y2);

    // Export Model Info
    std::vector<int> txRate;
    std::vector<int> rxRate;
    _model->Export(_process, _zoomFactor, txRate, rxRate);

    // - Calculate some Parameters of the Graph
    int graphWidth = x2 - x1;
    int iFactor = 
        (_zoomFactor == ZOOM_1S)  ? 1 :
        (_zoomFactor == ZOOM_10S) ? 10 : 60;

    int startIndex = 
        (graphWidth > (int) rxRate.size() * 2 - 2) ? 0 : rxRate.size() - graphWidth / 2 - 1;
    int endIndex = rxRate.size();

    // - Calculate the Max Rate
    int maxRate = 0;

    for(int i = startIndex; i < endIndex; i++)
    {
        if (txRate[i] > maxRate )
        {
            maxRate = txRate[i];
        }

        if (rxRate[i] > maxRate )
        {
            maxRate = rxRate[i];
        }
    }

    maxRate = 
        (_zoomFactor == ZOOM_1S)  ? maxRate :
        (_zoomFactor == ZOOM_10S) ? maxRate / 10 : maxRate / 60;

    maxRate = maxRate / 1024 + 1; // Unit is now KB/s

    // Decide Scale
    //    1.  [     0 KB/s,     10 KB/s]     10,      8,      6,     4,     2,    0
    //
    //    2.  (    10 KB/s,     20 KB/s]     20,     15,     10,     5,     0
    //    3.  (    20 KB/s,     40 KB/s]     40,     30,     20,    10,     0
    //    4.  (    40 KB/s,     50 KB/s]     50,     40,     30,    20,    10,    0
    //    5.  (    50 KB/s,     80 KB/s]     80,     60,     40,    20,     0
    //    6.  (    80 KB/s,    100 KB/s]    100,     80,     60,    40,    20,    0
    //
    //    7.  (   100 KB/s,    200 KB/s]    200,    150,    100,    50,     0
    //    8.  (   200 KB/s,    400 KB/s]    400,    300,    200,   100,     0
    //    9.  (   400 KB/s,    500 KB/s]    500,    400,    300,   200,   100,    0
    //    10. (   500 KB/s,    800 KB/s]    800,    600,    400,   200,     0
    //    11. (   800 KB/s,   1000 KB/s]   1000,    800,    600,   400,   200,    0
    //
    //    12. (  1000 KB/s,   2000 KB/s]   2000,   1500,   1000,   500,     0
    //    13. (  2000 KB/s,   4000 KB/s]   4000,   3000,   2000,  1000,     0
    //    14. (  4000 KB/s,   5000 KB/s]   5000,   4000,   3000,  2000,  1000,    0
    //    15. (  5000 KB/s,   8000 KB/s]   8000,   6000,   4000,  2000,     0
    //    16. (  8000 KB/s,  10000 KB/s]  10000,   8000,   6000,  4000,  2000,    0
    //
    //    17. ( 10000 KB/s,  20000 KB/s]  20000,  15000,  10000,  5000,     0
    //    18. ( 20000 KB/s,  40000 KB/s]  40000,  30000,  20000, 10000,     0
    //    19. ( 40000 KB/s,  50000 KB/s]  50000,  40000,  30000, 20000, 10000,    0
    //    20. ( 50000 KB/s,  80000 KB/s]  80000,  60000,  40000, 20000,     0
    //    21. ( 80000 KB/s, 100000 KB/s] 100000,  80000,  60000, 40000, 20000,    0

    //    22. (100000 KB/s, 200000 KB/s] 200000, 150000, 100000, 50000,     0
    //    ( ...Higher rates are not currently supported )
    const int rates[22] = {
        10, 
        20, 40, 50, 80, 100, 
        200, 400, 500, 800, 1000, 
        2000, 4000, 5000, 8000, 10000, 
        20000, 40000, 50000, 80000, 100000,
        200000
    };
    const int segments[22] = {
        5, 
        4, 4, 5, 4, 5, 
        4, 4, 5, 4, 5, 
        4, 4, 5, 4, 5, 
        4, 4, 5, 4, 5,
        4
    };

    int numSegment = 0;
    int scaleRate;

    for(int i = 0; i < 22; i++)
    {
        if (maxRate <= rates[i] )
        {
            scaleRate = rates[i];
            numSegment = segments[i];
            break;
        }
    }

    if (numSegment == 0) // Higher than 5000 KB/s
    {
        scaleRate = rates[22 - 1];
        numSegment = segments[22 - 1];
    }

    // Draw Scale

    // - Y Axis
    TCHAR yAxisText[16];

    SelectObject(_hdcBuf, _hEnglishFont);
    SetTextColor(_hdcBuf, RGB(0, 0, 0));
    SetTextAlign(_hdcBuf, TA_RIGHT);

    if (numSegment == 4 )
    {
        for(int i = 0; i < 5; i++)
        {
            _stprintf_s(yAxisText, _countof(yAxisText), TEXT("%d"), scaleRate * (4 - i) / 4);
            TextOut(_hdcBuf, x1 - 6, y1 + (y2 - y1 - 4) * i / 4 - 3, yAxisText, _tcslen(yAxisText));
        }
    }
    else if (numSegment == 5 )
    {
        for(int i = 0; i < 6; i++)
        {
            _stprintf_s(yAxisText, _countof(yAxisText), TEXT("%d"), scaleRate * (5 - i) / 5);
            TextOut(_hdcBuf, x1 - 6, y1 + (y2 - y1 - 4) * i / 5 - 3, yAxisText, _tcslen(yAxisText));
        }
    }

    // - X Axis
    //      A labels is 120 pixels to one another.
    //      For ZOOM_1S , it's 1 minute
    //      For ZOOM_10S, it's 10 minutes
    //      For ZOOM_60S, is's 1 hour
    TCHAR xAxisTextArray[3][5][16] = 
    {
        {TEXT("00:00:00"), TEXT("00:01:00"), TEXT("00:02:00"), TEXT("00:03:00"), TEXT("00:04:00")},
        {TEXT("00:00:00"), TEXT("00:10:00"), TEXT("00:20:00"), TEXT("00:30:00"), TEXT("00:40:00")},
        {TEXT("00:00:00"), TEXT("01:00:00"), TEXT("02:00:00"), TEXT("03:00:00"), TEXT("04:00:00")},
    };
    int index = 
        (_zoomFactor == ZOOM_1S) ? 0 : 
        (_zoomFactor == ZOOM_10S) ? 1 : 2;

    for(int i = 0; i < 5; i++)
    {
        TextOut(_hdcBuf, x2 - 120 * i, y2, xAxisTextArray[index][i], 8);
    }

    // - Scale Line
    SetDCPenColor(_hdcBuf, RGB(0xCC, 0xCC, 0xCC));

    if (numSegment == 4 )
    {
        for(int i = 1; i < 5 - 1; i++)
        {
            MoveToEx(_hdcBuf, x1 + 1, y1 + (y2 - y1) * i / 4, 0);
            LineTo(_hdcBuf, x2 - 1, y1 + (y2 - y1) * i / 4);
        }
    }
    else if (numSegment == 5 )
    {
        for(int i = 1; i < 6 - 1; i++)
        {
            MoveToEx(_hdcBuf, x1 + 1, y1 + (y2 - y1) * i / 5, 0);
            LineTo(_hdcBuf, x2 - 1, y1 + (y2 - y1) * i / 5);
        }
    }

    SetDCPenColor(_hdcBuf, RGB(0, 0, 0));

    // Draw Rate

    // - Rx Rate
    SetDCPenColor(_hdcBuf, RGB(0x31, 0x77, 0xC1));

    for(int i = startIndex; i < endIndex - 2; i++)
    {
        int yPos1 = ((y2 - y1) * (__int64)rxRate[i]) / (1024 * scaleRate * iFactor);
        int yPos2 = ((y2 - y1) * (__int64)rxRate[i + 1]) / (1024 * scaleRate * iFactor);

        if (_smoothFactor == SMOOTH_2X )
        {
            if (i - startIndex >= 1 ) // One More Point Taken into Consideration
            {
                int yPos1_1 = (y2 - y1) * (__int64)rxRate[i - 1] / (1024 * scaleRate * iFactor);
                int yPos2_1 = (y2 - y1) * (__int64)rxRate[i] / (1024 * scaleRate * iFactor);

                // Calculate Average
                yPos1 = (yPos1 + yPos1_1) / 2;
                yPos2 = (yPos2 + yPos2_1) / 2;
            }
        }
        else if (_smoothFactor == SMOOTH_4X )
        {
            if (i - startIndex >= 3 ) // Three More Points
            {
                int yPos1_1 = (y2 - y1) * (__int64)rxRate[i - 1] / (1024 * scaleRate * iFactor);
                int yPos2_1 = (y2 - y1) * (__int64)rxRate[i] / (1024 * scaleRate * iFactor);

                int yPos1_2 = (y2 - y1) * (__int64)rxRate[i - 2] / (1024 * scaleRate * iFactor);
                int yPos2_2 = (y2 - y1) * (__int64)rxRate[i - 1] / (1024 * scaleRate * iFactor);

                int yPos1_3 = (y2 - y1) * (__int64)rxRate[i - 3] / (1024 * scaleRate * iFactor);
                int yPos2_3 = (y2 - y1) * (__int64)rxRate[i - 2] / (1024 * scaleRate * iFactor);

                // Calc Average
                yPos1 = (yPos1 + yPos1_1 + yPos1_2 + yPos1_3) / 4;
                yPos2 = (yPos2 + yPos2_1 + yPos2_2 + yPos2_3) / 4;
            }
        }

        yPos1 = (yPos1 == 0) ? 1 : yPos1;
        yPos2 = (yPos2 == 0) ? 1 : yPos2;

        MoveToEx(_hdcBuf, x2 - (endIndex - i) * 2 + 3, y2 - 1 - yPos1, 0);
        LineTo(_hdcBuf, x2 - (endIndex - i - 1) * 2 + 3, y2 - 1 - yPos2);
    }

    // - Tx Rate
    SetDCPenColor(_hdcBuf, RGB(0xDF, 0x00, 0x24));

    for(int i = startIndex; i < endIndex - 2; i++)
    {
        int yPos1 = ((y2 - y1) * (__int64)txRate[i]) / (1024 * scaleRate * iFactor);
        int yPos2 = ((y2 - y1) * (__int64)txRate[i + 1]) / (1024 * scaleRate * iFactor);

        if (_smoothFactor == SMOOTH_2X )
        {
            if (i - startIndex >= 1 ) // One More Point Taken into Consideration
            {
                int yPos1_1 = (y2 - y1) * (__int64)txRate[i - 1] / (1024 * scaleRate * iFactor);
                int yPos2_1 = (y2 - y1) * (__int64)txRate[i] / (1024 * scaleRate * iFactor);

                // Calculate Average
                yPos1 = (yPos1 + yPos1_1) / 2;
                yPos2 = (yPos2 + yPos2_1) / 2;
            }
        }
        else if (_smoothFactor == SMOOTH_4X )
        {
            if (i - startIndex >= 3 ) // Three More Points
            {
                int yPos1_1 = (y2 - y1) * (__int64)txRate[i - 1] / (1024 * scaleRate * iFactor);
                int yPos2_1 = (y2 - y1) * (__int64)txRate[i] / (1024 * scaleRate * iFactor);

                int yPos1_2 = (y2 - y1) * (__int64)txRate[i - 2] / (1024 * scaleRate * iFactor);
                int yPos2_2 = (y2 - y1) * (__int64)txRate[i - 1] / (1024 * scaleRate * iFactor);

                int yPos1_3 = (y2 - y1) * (__int64)txRate[i - 3] / (1024 * scaleRate * iFactor);
                int yPos2_3 = (y2 - y1) * (__int64)txRate[i - 2] / (1024 * scaleRate * iFactor);

                // Calculate Average
                yPos1 = (yPos1 + yPos1_1 + yPos1_2 + yPos1_3) / 4;
                yPos2 = (yPos2 + yPos2_1 + yPos2_2 + yPos2_3) / 4;
            }
        }

        yPos1 = (yPos1 == 0) ? 1 : yPos1;
        yPos2 = (yPos2 == 0) ? 1 : yPos2;

        MoveToEx(_hdcBuf, x2 - (endIndex - i) * 2 + 3, y2 - 1 - yPos1, 0);
        LineTo(_hdcBuf, x2 - (endIndex - i - 1) * 2 + 3, y2 - 1 - yPos2);
    }

    SetDCPenColor(_hdcBuf, RGB(0, 0, 0));

    // Draw Box for Graph Again 
    //     Otherwise there will be an unexpected offset on last x.
    //     To be fixed  later.
    SelectObject(_hdcBuf, GetStockObject(NULL_BRUSH));
    Rectangle(_hdcBuf, x1, y1, x2, y2);
    SelectObject(_hdcBuf, GetStockObject(WHITE_BRUSH));

    // Draw Legend
    // -----------------------------------
    // |  -----------                    |
    // |  |  -----  |     Tx Rate        |
    // |  -----------                    |
    // |  -----------                    |
    // |  |  -----  |     Rx Rate        |
    // |  -----------                    |
    // -----------------------------------
    int legendX1 = x1 + 10;
    int legendY1 = y1 + 7;
    int legendX2 = legendX1 + 84;
    int legendY2 = legendY1 + 44;

    int legendBoxTxX1 = legendX1 + 6;
    int legendBoxTxY1 = legendY1 + 6;
    int legendBoxTxX2 = legendBoxTxX1 + 20;
    int legendBoxTxY2 = legendBoxTxY1 + 13;

    int legendBoxRxX1 = legendBoxTxX1;
    int legendBoxRxY1 = legendBoxTxY2 + 6;
    int legendBoxRxX2 = legendBoxTxX2;
    int legendBoxRxY2 = legendBoxRxY1 + 13;

    int legendTxX1 = legendBoxTxX1 + 4;
    int legendTxX2 = legendBoxTxX2 - 4;
    int legendTxY1 = legendBoxTxY1 + 6;
    int legendTxY2 = legendBoxTxY1 + 7;

    int legendRxX1 = legendBoxRxX1 + 4;
    int legendRxX2 = legendBoxRxX2 - 4;
    int legendRxY1 = legendBoxRxY1 + 6;
    int legendRxY2 = legendBoxRxY1 + 7;

    Rectangle(_hdcBuf, legendX1, legendY1, legendX2, legendY2);

    Rectangle(_hdcBuf, legendBoxTxX1, legendBoxTxY1, legendBoxTxX2, legendBoxTxY2);
    Rectangle(_hdcBuf, legendBoxRxX1, legendBoxRxY1, legendBoxRxX2, legendBoxRxY2);

    SelectObject(_hdcBuf, _hShellDlgFont);
    SetTextAlign(_hdcBuf, TA_LEFT | TA_BASELINE);

    TextOut(_hdcBuf, legendBoxTxX2 + 5, legendBoxTxY1 + 10, 
        Language::GetString(IDS_RTVIEW_TXRATE), _tcslen(Language::GetString(IDS_RTVIEW_TXRATE)));
    TextOut(_hdcBuf, legendBoxRxX2 + 5, legendBoxRxY1 + 10, 
        Language::GetString(IDS_RTVIEW_RXRATE), _tcslen(Language::GetString(IDS_RTVIEW_RXRATE)));

    SetDCPenColor(_hdcBuf, RGB(0xDF, 0x00, 0x24));
    Rectangle(_hdcBuf, legendTxX1, legendTxY1, legendTxX2, legendTxY2);
    
    SetDCPenColor(_hdcBuf, RGB(0x31, 0x77, 0xC1));
    Rectangle(_hdcBuf, legendRxX1, legendRxY1, legendRxX2, legendRxY2);

    SetDCPenColor(_hdcBuf, RGB(0, 0, 0));

    // Smooth Factor
    int smoothX = legendX2 + 4;
    int smoothY = legendY1 + 2;
    int smoothD = 13;

    SelectObject(_hdcBuf, _hEnglishFont);
    SetTextAlign(_hdcBuf, TA_LEFT | TA_TOP);

    TextOut(_hdcBuf, smoothX, smoothY + 0 * smoothD, TEXT("1X"), 2);
    TextOut(_hdcBuf, smoothX, smoothY + 1 * smoothD, TEXT("2X"), 2);
    TextOut(_hdcBuf, smoothX, smoothY + 2 * smoothD, TEXT("4X"), 2);

    SetBkColor(_hdcBuf, RGB(0xDD, 0xDD, 0xDD));

    if (_smoothFactor == SMOOTH_1X )
    {
        TextOut(_hdcBuf, smoothX, smoothY + 0 * smoothD, TEXT("1X"), 2);
    }
    else if (_smoothFactor == SMOOTH_2X )
    {
        TextOut(_hdcBuf, smoothX, smoothY + 1 * smoothD, TEXT("2X"), 2);
    }
    else if (_smoothFactor == SMOOTH_4X )
    {
        TextOut(_hdcBuf, smoothX, smoothY + 2 * smoothD, TEXT("4X"), 2);
    }

    SetBkColor(_hdcBuf, RGB(0xFF, 0xFF, 0xFF));

    // Zoom Factor
    int zoomX = x1 + 1;
    int zoomY = y2 + 0;

    TextOut(_hdcBuf, zoomX + 0,  zoomY, TEXT("1X"), 2);
    TextOut(_hdcBuf, zoomX + 16, zoomY, TEXT("10X"), 3);
    TextOut(_hdcBuf, zoomX + 37, zoomY, TEXT("60X"), 3);

    SetBkColor(_hdcBuf, RGB(0xDD, 0xDD, 0xDD));

    if (_zoomFactor == ZOOM_1S )
    {
        TextOut(_hdcBuf, zoomX + 0, zoomY, TEXT("1X"), 2);
    }
    else if (_zoomFactor == ZOOM_10S )
    {
        TextOut(_hdcBuf, zoomX + 16, zoomY, TEXT("10X"), 3);
    }
    else if (_zoomFactor == ZOOM_60S )
    {
        TextOut(_hdcBuf, zoomX + 37, zoomY, TEXT("60X"), 3);
    }

    SetBkColor(_hdcBuf, RGB(0xFF, 0xFF, 0xFF));

    // Process Name
    SelectObject(_hdcBuf, _hProcessFont);
    SetTextColor(_hdcBuf, RGB(128, 128, 128));
    if (_process == -1)
    {
        TextOut(_hdcBuf, legendX1 + 4, legendY2 + 2, 
            Language::GetString(IDS_ALL_PROCESS), _tcslen(Language::GetString(IDS_ALL_PROCESS)));
    }
    else
    {
        TCHAR buf[MAX_PATH];
        ProcessModel::GetProcessName(_process, buf, MAX_PATH);
        TextOut(_hdcBuf, legendX1 + 4, legendY2 + 2, buf, _tcslen(buf));
    }

    // Write to Screen
    BitBlt(_hdcTarget, 0, 0, _width, _height, _hdcBuf, 0, 0, SRCCOPY);
}

INT_PTR RealtimeView::DlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
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
        }

        // - Font
        _hEnglishFont  = Utils::MyCreateFont(TEXT("Arial"), 12, 0, false);
        _hShellDlgFont = Utils::MyCreateFont(TEXT("MS Shell Dlg 2"), 14, 0, false);
        _hProcessFont  = Utils::MyCreateFont(TEXT("MS Shell Dlg 2"), 14, 0, true);

        SelectObject(_hdcBuf, _hEnglishFont); 

        // - Pen
        SelectObject(_hdcBuf, GetStockObject(DC_PEN));

        // Start Timer
        SetTimer(hWnd, 0, 1000, RealtimeView::TimerProc);
    }
    else if (uMsg == WM_CLOSE )
    {
        KillTimer(hWnd, 0);

        ReleaseDC(hWnd, _hdcTarget);

        DeleteObject(_hEnglishFont);
        DeleteObject(_hShellDlgFont);
        DeleteObject(_hProcessFont);

        DestroyWindow(hWnd);
    }
    else if (uMsg == WM_PAINT )
    {
        PAINTSTRUCT stPS;
        BeginPaint(hWnd, &stPS);

        DrawGraph();

        EndPaint(hWnd, &stPS);
    }
    else if (uMsg == WM_LBUTTONDOWN )
    {
        int x = (lParam & 0xFFFF);
        int y = (lParam & 0xFFFF0000) >> 16;

        // Smooth Factor
        if (x >= 127 && x <= 140 )
        {
            if (y >= 19 && y < 32 )
            {
                _smoothFactor = SMOOTH_1X;
                DrawGraph();
            }
            else if (y >= 32 && y < 45 )
            {
                _smoothFactor = SMOOTH_2X;
                DrawGraph();
            }
            else if (y >= 45 && y < 58 )
            {
                _smoothFactor = SMOOTH_4X;
                DrawGraph();
            }
        }

        // Get Width and Height
        RECT stRect;
        GetClientRect(_hWnd, &stRect);

        int _width  = stRect.right - stRect.left;
        int _height = stRect.bottom - stRect.top;

        // Zoom Factor
        int x1 = 34;
        int y1 = 10;
        int x2 = _width - 10;
        int y2 = _height - 26;

        int zoomX = x1 + 1;
        int zoomY = y2 + 0;

        if (y >= zoomY && y <= zoomY + 12 )
        {
            if (x >= zoomX && x <= zoomX + 12 )
            {
                _zoomFactor = ZOOM_1S;
                DrawGraph();
            }
            else if (x >= zoomX + 16 && x <= zoomX + 33 )
            {
                _zoomFactor = ZOOM_10S;
                DrawGraph();
            }
            else if (x >= zoomX + 37 && x <= zoomX + 54 )
            {
                _zoomFactor = ZOOM_60S;
                DrawGraph();
            }
        }
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
