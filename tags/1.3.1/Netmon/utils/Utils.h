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
	static int InsertProcess(const TCHAR *name);
	static int InsertProcessActivity(int puid, int startTime, int endTime);
	static void UpdateProcessActivity(int pauid, int endTime);
	static bool GetProcessName(int puid, TCHAR *buf, int len);
	static void InsertPacket(PacketInfoEx *pi);
	static void DeleteAllPackets();

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

	// Tab
	static void TabInit(HWND hTab, int numTabs, ...);
	static void TabSetText(HWND hTab, int numTabs, ...);

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
	static void SetMenuString(HMENU hMnu, UINT uPosition, UINT uFlags, UINT_PTR uIDNewItem, LPCTSTR lpNewItem);

	// Process
	static BOOL StartProcessAndWait(const TCHAR *szFile, const TCHAR *szParam, int *pExitCode, BOOL bRunAs);

	// File & Directory
	static void GetSomeFilePathNameInCurrentDir(TCHAR *buf, int cchLen, const TCHAR *szFileName);
};

#endif