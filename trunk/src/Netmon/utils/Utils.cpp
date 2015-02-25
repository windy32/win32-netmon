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
#include "Utils.h"

#include "SQLite.h"

// ListView
void Utils::InnerListViewInsert(HWND hList, int index, int numColumns, va_list argList)
{
    const int MAX_ARG = 32;

    // Get arguments
    TCHAR *szColumns[MAX_ARG];

    for (int i = 0; i < numColumns; i++)
    {
        szColumns[i] = va_arg(argList, TCHAR *);
    }

    // Insert
    LVITEM stItem;

    // - Initialize LVITEM
    stItem.mask = LVIF_TEXT;
    stItem.iItem = index;
    stItem.iSubItem = 0;
    stItem.pszText = szColumns[0];

    // - Insert First Item
    stItem.iItem = ListView_InsertItem(hList, &stItem);

    // - Insert Sub Items
    for (int i = 1; i < numColumns; i++)
    {
        stItem.pszText = szColumns[i];
        stItem.iSubItem = i;

        ListView_SetItem(hList, &stItem);
    }
}

// Database
int Utils::GetProcessUid(const TCHAR *name)
{
    TCHAR command[256];

    // Build Command
    _stprintf_s(command, _countof(command), 
        TEXT("Select UID From Process Where Name = \'%s\';"), name);

    // Build SQLiteRow Object
    SQLiteRow row;
    row.InsertType(SQLiteRow::TYPE_INT32);

    // Select
    if (SQLite::Select(command, &row))
    {
        return row.GetDataInt32(0);
    }

    return -1;
}

int Utils::InsertProcess(const TCHAR *name, const TCHAR *fullPath)
{
    TCHAR command[512];

    // Build Command
    _stprintf_s(command, _countof(command), 
        TEXT("Insert Into Process(Name, FullPath) Values(\'%s\', '%s');"), name, fullPath);

    // Insert
    SQLite::Exec(command, false);

    // Return the UID of the inserted item
    return (int)SQLite::GetLastInsertRowId();
}

void Utils::UpdateFullPath(int puid, const TCHAR *fullPath)
{
    TCHAR command[512];

    // Build Command
    _stprintf_s(command, _countof(command), 
        TEXT("Update Process Set FullPath = \'%s\' Where UID = %d;"), fullPath, puid);

    // Insert
    SQLite::Exec(command, false);
}

bool Utils::GetProcessName(int puid, TCHAR *buf, int len)
{
    TCHAR command[256];

    // Build Command
    _stprintf_s(command, _countof(command), 
        TEXT("Select Name From Process Where UID = %d;"), puid);

    // Build SQLiteRow Object
    SQLiteRow row;
    row.InsertType(SQLiteRow::TYPE_STRING);

    // Select
    if (SQLite::Select(command, &row))
    {
        _tcscpy_s(buf, len, row.GetDataStr(0));
        return true;
    }

    return false;
}

void Utils::InsertPacket(PacketInfoEx *pi)
{
    TCHAR command[256];

    // Build Command
    __int64 i64time = ((__int64)(pi->time_s) << 32) + (__int64)(pi->time_us);

    _stprintf_s(command, _countof(command), 
        TEXT("Insert Into Packet(ProcessUID, Direction, ")
        TEXT("NetProtocol, TraProtocol, Size, Time, Port) ")
        TEXT("Values(%d, %d, %d, %d, %d, %I64d, %d);"), 
        pi->puid, pi->dir, pi->networkProtocol, pi->trasportProtocol, 
        pi->size, i64time, pi->remote_port);

    // Insert
    SQLite::Exec(command, true);
}

// Time
/*
int Utils::GetNumDays(int exMonth)
{
    int iYear  = 1970 + exMonth / 12;
    int iMonth = exMonth % 12; // 0 to 11;

    int iDays[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

    // Leap Year
    if ((iYear % 4 == 0) && (iYear % 100 != 0 || iYear % 400 == 0))
    {
        iDays[1] = 29;
    }

    return iDays[iMonth];
}

int Utils::GetExMonth()
{
    time_t tTime = time(0);
    struct tm tmTime;
    
    localtime_s(&tmTime, &tTime);

    return tmTime.tm_mon + (tmTime.tm_year - 70) * 12;
}

int Utils::GetExMonthByDate(int date)
{
    return date >> 16;
}

int Utils::GetMdayByDate(int date)
{
    return date & 0xFFFF;
}

int Utils::GetExMonth(time_t tTime)
{
    struct tm tmTime;
    
    localtime_s(&tmTime, &tTime);

    return tmTime.tm_mon + (tmTime.tm_year - 70) * 12;
}

int Utils::GetDay()
{
    time_t tTime = time(0);
    struct tm tmTime;
    
    localtime_s(&tmTime, &tTime);

    return tmTime.tm_mday; // 1 to 31
}

int Utils::GetDay(time_t tTime)
{
    struct tm tmTime;
    
    localtime_s(&tmTime, &tTime);

    return tmTime.tm_mday; // 1 to 31
}

int Utils::GetWeekDay(int exMonth, int mday)
{
    // Build the tm Structure
    time_t tTime = time(0);
    struct tm tmTime;
    
    localtime_s(&tmTime, &tTime);

    // Reset it's month and mday
    tmTime.tm_year = 70 + exMonth / 12;
    tmTime.tm_mon =  exMonth % 12;
    tmTime.tm_mday = mday; // 1 to 31

    // Convert to time_t Again
    time_t targetTime = mktime(&tmTime);

    // Convert to tm Structure Again
    localtime_s(&tmTime, &targetTime);

    return tmTime.tm_wday; // 0 to 6
}
*/

// ListView
void Utils::ListViewInit(HWND hList, BOOL bCheckBox, int numColumns, ...)
{
    const int MAX_ARG = 32;

    // Get arguments
    TCHAR *szHeaders[MAX_ARG];
    int iHeaderWidths[MAX_ARG];

    va_list argList;
    va_start(argList, numColumns);

    for(int i = 0; i < numColumns; i++)
    {
        szHeaders[i] = va_arg(argList, TCHAR *);
    }

    for(int i = 0; i < numColumns; i++)
    {
        iHeaderWidths[i] = va_arg(argList, int);
    }

    va_end(argList);

    // Init ListView
    if (bCheckBox )
    {
        ListView_SetExtendedListViewStyle(hList, LVS_EX_CHECKBOXES | LVS_EX_FULLROWSELECT);
    }
    else
    {
        ListView_SetExtendedListViewStyle(hList, LVS_EX_FULLROWSELECT);
    }

    // Modern Style is disabled
    // SetWindowTheme(hList, L"Explorer", NULL);

    // Initialize the LVCOLUMN structure.
    LVCOLUMN stLVC;
    stLVC.mask = LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;

    // Insert Columns
    for(int i = 0; i < numColumns; i++)
    {
        stLVC.cx = iHeaderWidths[i];
        stLVC.iSubItem = i;
        stLVC.pszText = szHeaders[i];

        ListView_InsertColumn(hList, i, &stLVC);
    }
}

void Utils::ListViewSetColumnText(HWND hList, int numColumns, ...)
{
    va_list argList;
    va_start(argList, numColumns);

    // Initialize the LVCOLUMN structure.
    LVCOLUMN lvc;
    lvc.mask = LVCF_TEXT;

    // Set column text
    for(int i = 0; i < numColumns; i++)
    {
        lvc.pszText = va_arg(argList, TCHAR *);
        ListView_SetColumn(hList, i, &lvc);
    }

    va_end(argList);
}

void Utils::ListViewClear(HWND hList)
{
    ListView_DeleteAllItems(hList);
}

void Utils::ListViewInsert(HWND hList, int index, int numColumns, ...)
{
    va_list argList;
    va_start(argList, numColumns);

    InnerListViewInsert(hList, index, numColumns, argList);

    va_end(argList);
}

void Utils::ListViewAppend(HWND hList, int numColumns, ...)
{
    va_list argList;
    va_start(argList, numColumns);

    InnerListViewInsert(hList, INT_MAX, numColumns, argList);

    va_end(argList);
}

void Utils::ListViewUpdate(HWND hList, int index, int numColumns, ...)
{
    const int MAX_ARG = 32;

    // Get arguments
    BOOL bUpdate[MAX_ARG];
    TCHAR *szColumns[MAX_ARG];

    va_list argList;
    va_start(argList, numColumns);

    for(int i = 0; i < numColumns; i++)
    {
        bUpdate[i] = va_arg(argList, BOOL);
    }

    for(int i = 0; i < numColumns; i++)
    {
        szColumns[i] = va_arg(argList, TCHAR *);
    }

    va_end(argList);

    // Update
    LVITEM stItem;

    // - Initialize LVITEM
    stItem.mask = LVIF_TEXT;
    stItem.iItem = index;
    stItem.iSubItem = 0;
    stItem.pszText = szColumns[0];

    // - Update Sub Items
    for(int i = 0; i < numColumns; i++)
    {
        if (bUpdate[i] )
        {
            stItem.pszText = szColumns[i];
            stItem.iSubItem = i;

            ListView_SetItem(hList, &stItem);
        }
    }
}

int Utils::ListViewGetRowCount(HWND hList)
{
    return ListView_GetItemCount(hList);
}

void Utils::ListViewGetText(HWND hList, int row, int column, TCHAR *buf, int cchLen)
{
    ListView_GetItemText(hList, row, column, buf, cchLen - 1);
}

int Utils::ListViewGetSelectedItemIndex(HWND hList)
{
    int count = ListView_GetItemCount(hList);
    for (int i = 0; i < count; i++)
    {
        if (ListView_GetItemState(hList, i, LVIS_SELECTED) == LVIS_SELECTED)
            return i;
    }
    return -1;
}

// Tab
void Utils::TabInit(HWND hTab, int numTabs, ...)
{
    va_list argList;
    va_start(argList, numTabs);

    // Initialize the TCITEM Structure
    TCITEM tci;
    tci.mask = TCIF_TEXT; 
 
    // Insert Tabs
    for (int i = 0; i < numTabs; i++)
    {
        tci.pszText = va_arg(argList, TCHAR *);

        TabCtrl_InsertItem(hTab, i, &tci); 
    }

    va_end(argList);
}

void Utils::TabInit(HWND hTab, int numTabs, const TCHAR *names[])
{
    for (int i = 0; i < numTabs; i++)
    {
        TCITEM tci;
        tci.mask = TCIF_TEXT;
        tci.pszText = (TCHAR *)names[i];
        TabCtrl_InsertItem(hTab, i, &tci);
    }
}

void Utils::TabSetText(HWND hTab, int numTabs, ...)
{
    va_list argList;
    va_start(argList, numTabs);

    // Initialize the TCITEM Structure
    TCITEM tci;
    tci.mask = TCIF_TEXT; 
 
    // Insert Tabs
    for (int i = 0; i < numTabs; i++) 
    {
        tci.pszText = va_arg(argList, TCHAR *);

        TabCtrl_SetItem(hTab, i, &tci); 
    }

    va_end(argList);
}

void Utils::TabSetText(HWND hTab, int numTabs, const TCHAR *names[])
{
    for (int i = 0; i < numTabs; i++)
    {
        TCITEM tci;
        tci.mask = TCIF_TEXT; 
        tci.pszText = (TCHAR *)names[i];
        TabCtrl_SetItem(hTab, i, &tci);
    }
}

// GDI
HFONT Utils::MyCreateFont(const TCHAR *name, int height, int width, bool bold)
{
    LOGFONT stLogFont;
    RtlZeroMemory(&stLogFont, sizeof(stLogFont));

    stLogFont.lfHeight = height;
    stLogFont.lfWidth = width;
    stLogFont.lfEscapement = 0;
    stLogFont.lfOrientation = 0;
    stLogFont.lfWeight = (bold ? FW_BOLD : FW_REGULAR);
    stLogFont.lfItalic = FALSE;
    stLogFont.lfUnderline = FALSE;
    stLogFont.lfStrikeOut = FALSE;
    _tcscpy_s(stLogFont.lfFaceName, _countof(stLogFont.lfFaceName), name);

    return CreateFontIndirect(&stLogFont); 
}

// Math
double Utils::Log(double value, double base)
{
    return log(value) / log(base);
}

double Utils::Exp(double value, double base)
{
    return exp(value * log(base));
}

// Codec
int Utils::Utf16ToUtf8(const TCHAR *src, char *dst, int len)
{
    return WideCharToMultiByte(CP_UTF8, 0, src, -1, dst, len, 0, 0);
}

int Utils::Utf8ToUtf16(const char *src, TCHAR *dst, int len)
{
    return MultiByteToWideChar(CP_UTF8, 0, src, -1, dst, len);
}

int Utils::AnsiToUtf16(const char *src, TCHAR *dst, int len)
{
    return MultiByteToWideChar(CP_ACP, 0, src, -1, dst, len);
}

// Debug
void Utils::DbgPrint(const TCHAR *format, ...)
{
    TCHAR buf[1024];
    va_list argList;
    va_start(argList, format);

    _vstprintf_s(buf, _countof(buf), format, argList);
    OutputDebugString(buf);

    va_end(argList);
}

// Version
void Utils::GetVersionString(TCHAR *buf, int cchLen)
{
    TCHAR szExe[MAX_PATH];
    BYTE *pVersionInfo = 0;
    INT iVersionInfoSize;
    VS_FIXEDFILEINFO *pFixedFileInfo = 0;
    UINT iFixedFileInfoSize;

    // Get full path name of "Netmon.exe"
    GetModuleFileName(0, szExe, MAX_PATH);

    // Get version info size
    iVersionInfoSize = GetFileVersionInfoSize(szExe, 0);

    // Get version info
    pVersionInfo = new BYTE[iVersionInfoSize];
    GetFileVersionInfo(szExe, 0, iVersionInfoSize, pVersionInfo);

    // Get fixed file info
    VerQueryValue(pVersionInfo, TEXT("\\"), (void **)&pFixedFileInfo, &iFixedFileInfoSize);

    // Set string
    _stprintf_s(buf, cchLen, TEXT("%d.%d.%d"), 
        HIWORD(pFixedFileInfo->dwFileVersionMS), 
        LOWORD(pFixedFileInfo->dwFileVersionMS),
        HIWORD(pFixedFileInfo->dwFileVersionLS));
}

// Menu
void Utils::SetMenuString(HMENU hMenu, UINT uItem, BOOL fByPosition, LPCTSTR szText)
{
    MENUITEMINFO info;
    info.cbSize = sizeof(info);
    info.fMask = MIIM_STRING;
    info.fType = MFT_STRING;
    info.dwTypeData = (LPTSTR)szText;

    SetMenuItemInfo(hMenu, uItem, fByPosition, &info);
}

// Process
BOOL Utils::StartProcess(const TCHAR *szFile, const TCHAR *szParam, BOOL bRunAs)
{
    SHELLEXECUTEINFO sei = { sizeof(SHELLEXECUTEINFO) };

    sei.fMask = SEE_MASK_NOCLOSEPROCESS;
    if (bRunAs )
    {
        sei.lpVerb = TEXT("runas");
    }
    sei.lpFile = szFile;
    sei.lpParameters = szParam;
    sei.nShow = SW_SHOWNORMAL;

    if (ShellExecuteEx(&sei) && sei.hProcess != 0)
    {
        CloseHandle(sei.hProcess);
        return TRUE;
    }

    return FALSE;
}

BOOL Utils::StartProcessAndWait(
    const TCHAR *szFile, const TCHAR *szParam, int *pExitCode, BOOL bRunAs)
{
    SHELLEXECUTEINFO sei = { sizeof(SHELLEXECUTEINFO) };
    DWORD dwExitCode;

    sei.fMask = SEE_MASK_NOCLOSEPROCESS;
    if (bRunAs )
    {
        sei.lpVerb = TEXT("runas");
    }
    sei.lpFile = szFile;
    sei.lpParameters = szParam;
    sei.nShow = SW_SHOWNORMAL;

    if (ShellExecuteEx(&sei) && sei.hProcess != 0 )
    {
        // Wait until finish
        WaitForSingleObject(sei.hProcess, INFINITE);

        // Get exit code
        GetExitCodeProcess(sei.hProcess, &dwExitCode);
        *pExitCode = dwExitCode;

        // Exit
        CloseHandle(sei.hProcess);
        return TRUE;
    }

    return FALSE;
}

// File & Directory
void Utils::GetFilePathInCurrentDir(TCHAR *buf, int cchLen, const TCHAR *szFileName)
{
    TCHAR szCurrentExe[MAX_PATH];
    TCHAR szCurrentDir[MAX_PATH];
    TCHAR *pFilePart;

    // Get full path name of Netmon.exe
    GetModuleFileName(0, szCurrentExe, MAX_PATH);

    // Get base directory
    GetFullPathName(szCurrentExe, MAX_PATH, szCurrentDir, &pFilePart);
    *pFilePart = TEXT('\0');

    // Get full path name of Netmon.ini
    _tcscpy_s(buf, cchLen, szCurrentDir);
    _tcscat_s(buf, cchLen, szFileName);
}
