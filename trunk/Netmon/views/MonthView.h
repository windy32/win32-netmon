#ifndef MONTH_VIEW_H
#define MONTH_VIEW_H

#include "NetView.h"

class MonthView : public NetView
{
protected:
	// The Item of a Process for a Month, 496 Bytes -------
	typedef struct tagMonthItem
	{
		__int64 dayTx[31]; // In Bytes
		__int64 dayRx[31];

		__int64 sumTx;
		__int64 sumRx;

		struct tagMonthItem()
		{
			RtlZeroMemory(dayTx, sizeof(dayTx));
			RtlZeroMemory(dayRx, sizeof(dayRx));

			sumTx = 0;
			sumRx = 0;
		}
	} MonthItem;

	// The Item of a Process ------------------------------
	typedef struct tagMtViewItem
	{
		static int firstMonth; // Jan 1970 = 0, Feb 1970 = 1 ...
		std::vector<MonthItem> months;

		TCHAR processName[MAX_PATH];

		struct tagMtViewItem()
		{
			RtlZeroMemory(processName, sizeof(processName));
		}
	} MtViewItem;

	// Items for All Processes ----------------------------
	static std::map<int, MtViewItem> _items;
	static int _curMonth;
	static MonthView * _this;
	static CRITICAL_SECTION _stCS;

protected:
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

protected:
	static void Fill();
	static void DrawGraph();
	static void WINAPI TimerProc(HWND hWnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime);

	static void SaveDatabase();
	static void InitDatabase();
	static void InitDatabaseCallback(SQLiteRow *row);

public:
	virtual void Init();
	virtual void End();
	virtual void InsertPacket(PacketInfoEx *pi);
	virtual void SetProcessUid(int puid, TCHAR *processName);

	virtual LRESULT DlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
};

#endif
