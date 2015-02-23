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

#ifndef UTILS_H
#define UTILS_H

#include "Packet.h"

// Utilities for database operations, time operations, etc.
class Utils
{
private:
    // ListView
    static void InnerListViewInsert(HWND hList, int index, int numColumns, va_list argList);

public:
    // Database operations
    static int GetProcessUid(const TCHAR *name);
    static int InsertProcess(const TCHAR *name, const TCHAR *fullPath);
    static void UpdateFullPath(int puid, const TCHAR *fullPath);
    static bool GetProcessName(int puid, TCHAR *buf, int len);
    static void InsertPacket(PacketInfoEx *pi);

    // Time operations
    static int GetNumDays(int exMonth);

    static int GetExMonth();
    static int GetExMonth(time_t tTime);

    static int GetExMonthByDate(int date);
    static int GetMdayByDate(int date);

    static int GetDay();
    static int GetDay(time_t tTime);
    static int GetWeekDay(int exMonth, int mday);

    // ListView
    static void ListViewInit(HWND hList, BOOL bCheckBox, int numColumns, ...);
    static void ListViewSetColumnText(HWND hList, int numColumns, ...);

    static void ListViewClear(HWND hList);
    static void ListViewInsert(HWND hList, int index, int numColumns, ...);
    static void ListViewAppend(HWND hList, int numColumns, ...);
    static void ListViewUpdate(HWND hList, int index, int numColumns, ...);

    static int ListViewGetRowCount(HWND hList);
    static void ListViewGetText(HWND hList, int row, int column, TCHAR *buf, int cchLen);
    static int ListViewGetSelectedItemIndex(HWND hList);

    // Tab
    static void TabInit(HWND hTab, int numTabs, ...);
    static void TabInit(HWND hTab, int numTabs, const TCHAR *names[]);
    static void TabSetText(HWND hTab, int numTabs, ...);
    static void TabSetText(HWND hTab, int numTabs, const TCHAR *names[]);

    // GDI operations
    static HFONT MyCreateFont(const TCHAR *name, int height, int width, bool bold);

    // Math
    static double Log(double value, double base);
    static double Exp(double value, double base);

    // Codec
    static int Utf16ToUtf8(const TCHAR *src, char *dst, int len);
    static int Utf8ToUtf16(const char *src, TCHAR *dst, int len);
    static int AnsiToUtf16(const char *src, TCHAR *dst, int len);

    // Debug
    static void DbgPrint(const TCHAR *format, ...);

    // Version
    static void GetVersionString(TCHAR *buf, int cchLen);

    // Menu
    static void SetMenuString(HMENU hMenu, UINT uItem, BOOL fByPosition, LPCTSTR szText);

    // Process
    static BOOL StartProcess(
        const TCHAR *szFile, const TCHAR *szParam, BOOL bRunAs);

    static BOOL StartProcessAndWait(
        const TCHAR *szFile, const TCHAR *szParam, int *pExitCode, BOOL bRunAs);

    // File & Directory
    static void GetFilePathInCurrentDir(TCHAR *buf, int cchLen, const TCHAR *szFileName);
};

#endif
