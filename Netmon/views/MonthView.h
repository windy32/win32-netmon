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
