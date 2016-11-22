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
#include "DlgAbout.h"

#include "../res/resource.h"

#include "../utils/Utils.h"
#include "../utils/Language.h"

///----------------------------------------------------------------------------------------------//
///                                    Global Variables                                          //
///----------------------------------------------------------------------------------------------//
extern HINSTANCE g_hInstance;
static WNDPROC   g_lpOldProcEdit;

///----------------------------------------------------------------------------------------------//
///                                    The WNDPROC That Makes An Edit Control Readonly           //
///----------------------------------------------------------------------------------------------//
static LRESULT CALLBACK MyProcEdit(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    if(uMsg != WM_CHAR && uMsg != WM_PASTE)
    {
        return CallWindowProc(g_lpOldProcEdit, hWnd, uMsg, wParam, lParam);
    }
    return 0;
}

///----------------------------------------------------------------------------------------------// 
///                                    L1 Message Handlers                                       //
///----------------------------------------------------------------------------------------------//
static void OnInitDialog(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
    // Load Icon
    HICON hIcon = LoadIcon(g_hInstance, MAKEINTRESOURCE(ICO_MAIN));
    SendMessage(hWnd, WM_SETICON, ICON_BIG, (LPARAM)hIcon);

    // Size Dialog
    SetWindowPos(hWnd, HWND_TOP, 140, 140, 569, 320, 0);

    // Init the Read-only Edit Control
    g_lpOldProcEdit = 
        (WNDPROC)SetWindowLong(GetDlgItem(hWnd, IDE_THIRD_PARTY), GWL_WNDPROC, (LONG)MyProcEdit);

    // Get Client Rectangle
    RECT stClientRect;

    GetClientRect(hWnd, &stClientRect);

    int x1 = stClientRect.left + 143;
    int y1 = stClientRect.top;
    int x2 = stClientRect.right;
    int y2 = stClientRect.bottom;

    // Move Controls
    SetWindowPos(GetDlgItem(hWnd, IDE_THIRD_PARTY), 
        HWND_TOP, x1 + 22, 100, x2 - x1 - 32, y2 - y1 - 144, 0);
    SetWindowPos(GetDlgItem(hWnd, IDB_CLOSE), 
        HWND_TOP, x2 - 90, y2 - 34, 80, 24, 0);

    // Set Bitmap
    SendMessage(GetDlgItem(hWnd, IDS_SIDEBAR), STM_SETIMAGE, 
        IMAGE_BITMAP, (LPARAM)LoadBitmap(g_hInstance, MAKEINTRESOURCE(IDB_ABOUT_SIDEBAR)));

    // Set close button text
    SetDlgItemText(hWnd, IDB_CLOSE, Language::GetString(IDS_ABOUT_CLOSE));

    // Set Third Party Infomation
    SetDlgItemText(hWnd, IDE_THIRD_PARTY, 
        TEXT("This software includes SQLite 3.6.7.3.\r\n")
        TEXT("\r\n")
        TEXT("The WinPcap Library is covered by the following license:\r\n")
        TEXT("\r\n")
        TEXT("   Copyright (c) 1993, 1994, 1995, 1996, 1997\r\n")
        TEXT("   The Regents of the University of California.\r\n")
        TEXT("   All rights reserved.\r\n")
        TEXT("\r\n")
        TEXT("   Redistribution and use in source and binary forms, with\r\n")
        TEXT("   or without modification, are permitted provided that the\r\n")
        TEXT("   following conditions are met:\r\n")
        TEXT("\r\n")
        TEXT("   1. Redistributions of source code must retain the above \r\n")
        TEXT("      copyright notice, this list of conditions and the \r\n")
        TEXT("      following disclaimer.\r\n")
        TEXT("\r\n")
        TEXT("   2. Redistributions in binary form must reproduce the \r\n")
        TEXT("      above copyright notice, this list of conditions and\r\n")
        TEXT("      the following disclaimer in the documentation and/or\r\n")
        TEXT("      other materials provided with the distribution.\r\n")
        TEXT("\r\n")
        TEXT("   3. All advertising materials mentioning features or use\r\n")
        TEXT("      of this software must display the following \r\n")
        TEXT("      acknowledgement:\r\n")
        TEXT("      This product includes software developed by the\r\n")
        TEXT("      Computer Systems Engineering Group at Lawrence \r\n")
        TEXT("      Berkeley Laboratory.\r\n")
        TEXT("\r\n")
        TEXT("   4. Neither the name of the University nor of the \r\n")
        TEXT("      Laboratory may be used to endorse or promote \r\n")
        TEXT("      products derived from this software without specific\r\n")
        TEXT("      prior written permission.\r\n")
        TEXT("\r\n")
        TEXT("   THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS\r\n")
        TEXT("   ``AS IS'' AND ANY EXPRESS OR IMPLIEDWARRANTIES, \r\n")
        TEXT("   INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF\r\n")
        TEXT("   MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE\r\n")
        TEXT("   DISCLAIMED. IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS\r\n")
        TEXT("   BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,\r\n")
        TEXT("   EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT\r\n")
        TEXT("   LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;\r\n")
        TEXT("   LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)\r\n")
        TEXT("   HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER\r\n")
        TEXT("   IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING \r\n")
        TEXT("   NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE\r\n")
        TEXT("   USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY\r\n")
        TEXT("   OF SUCH DAMAGE.\r\n"));
}

static void OnPaint(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
    PAINTSTRUCT stPS;
    BeginPaint(hWnd, &stPS);

    // Get Client Rectangle
    RECT stClientRect;
    GetClientRect(hWnd, &stClientRect);

    int x1 = stClientRect.left + 143;
    int y1 = stClientRect.top;
    int x2 = stClientRect.right;
    int y2 = stClientRect.bottom;

    // Create fonts
    HFONT hFontDefault;
    HFONT hFontTitle  = Utils::MyCreateFont(TEXT("MS Shell Dlg 2"), 14, 0, true);
    HFONT hFontTitle2 = Utils::MyCreateFont(TEXT("MS Shell Dlg 2"), 14, 0, true);
    HFONT hFontText   = Utils::MyCreateFont(TEXT("MS Shell Dlg 2"), 14, 0, false);

    // Get version string
    TCHAR szVersionNumber[64];
    TCHAR szVersion[64];
    const TCHAR *szVersionFormat = Language::GetString(IDS_ABOUT_NETMON);

    Utils::GetVersionString(szVersionNumber, 64);
    _stprintf_s(szVersion, 64, szVersionFormat, szVersionNumber);
    
    // Clear Background
    Rectangle(stPS.hdc, x1 - 1, y1 - 1, x2 + 1, y2 + 1);

    // Draw Text

    // - Netmon 1.2.1
    SetTextAlign(stPS.hdc, TA_LEFT | TA_BOTTOM);
    SetTextColor(stPS.hdc, RGB(0x17, 0x14, 0xA3));
    hFontDefault = (HFONT) SelectObject(stPS.hdc, hFontTitle);
    TextOut(stPS.hdc, x1 + 10, y1 + 24, szVersion, _tcslen(szVersion));

    // - Line
    MoveToEx(stPS.hdc, x1 + 10, y1 + 27, 0);
    LineTo(stPS.hdc, x2 - 10, y1 + 27);

    // - Copyright (C) ...
    // - All rights ...
    SetTextAlign(stPS.hdc, TA_LEFT | TA_TOP);
    SetTextColor(stPS.hdc, RGB(0x00, 0x00, 0x00));
    SelectObject(stPS.hdc, hFontText);

    TextOut(stPS.hdc, x1 + 22, y1 + 34, Language::GetString(IDS_ABOUT_COPYRIGHT),  
        _tcslen(Language::GetString(IDS_ABOUT_COPYRIGHT)));
    TextOut(stPS.hdc, x1 + 22, y1 + 48, Language::GetString(IDS_ABOUT_ALL_RIGHTS), 
        _tcslen(Language::GetString(IDS_ABOUT_ALL_RIGHTS)));

    // - Third parties
    SetTextAlign(stPS.hdc, TA_LEFT | TA_BOTTOM);
    SetTextColor(stPS.hdc, RGB(0x17, 0x14, 0xA3));
    SelectObject(stPS.hdc, hFontTitle2);
    TextOut(stPS.hdc, x1 + 10, y1 + 87, Language::GetString(IDS_ABOUT_THIRD_PARTIES), 
        _tcslen(Language::GetString(IDS_ABOUT_THIRD_PARTIES)));

    // - Line
    MoveToEx(stPS.hdc, x1 + 10, y1 + 90, 0);
    LineTo(stPS.hdc, x2 - 10, y1 + 90);

    SelectObject(stPS.hdc, hFontDefault);

    DeleteObject(hFontTitle);
    DeleteObject(hFontTitle2);
    DeleteObject(hFontText);

    EndPaint(hWnd, &stPS);
}

static void OnClose(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
    EndDialog(hWnd, 0);
}

static void OnCommand(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
    if( wParam == IDB_CLOSE )
    {
        EndDialog(hWnd, 0);
    }
}

///----------------------------------------------------------------------------------------------//
///                                    About Dialog Proc                                         //
///----------------------------------------------------------------------------------------------//
INT_PTR CALLBACK ProcDlgAbout(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
#define PROCESS_MSG(MSG, HANDLER) if(uMsg == MSG) { HANDLER(hWnd, wParam, lParam); return TRUE; }

    PROCESS_MSG(WM_INITDIALOG, OnInitDialog)
    PROCESS_MSG(WM_CLOSE,      OnClose)
    PROCESS_MSG(WM_COMMAND,    OnCommand)
    PROCESS_MSG(WM_PAINT,      OnPaint)

#undef PROCESS_MSG

    return FALSE;
}
