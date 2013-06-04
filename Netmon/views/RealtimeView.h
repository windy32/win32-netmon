#ifndef REALTIME_VIEW_H
#define REALTIME_VIEW_H

#include "NetView.h"
#include "RealtimeModel.h"

class RealtimeView : public NetView
{
protected:
	// Settings
	static enum ZoomFactor _zoomFactor;

	static enum SmoothFactor
	{
		SMOOTH_1X,
		SMOOTH_2X,
		SMOOTH_4X
	} _smoothFactor;

	// GDI Objects
	static HDC     _hdcTarget;
	static HDC     _hdcBuf;
	static HBITMAP _hbmpBuf;

	static HFONT   _hOldFont;
	static HFONT   _hEnglishFont;
	static HFONT   _hShellDlgFont;
	static HFONT   _hProcessFont;

	// Model object
	static RealtimeModel *_model;

protected:
	static void DrawGraph();
	static void WINAPI TimerProc(HWND hWnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime);

public:
	virtual void Init(RealtimeModel *model);
	virtual void End();
	virtual void SetProcessUid(int puid);

	virtual LRESULT DlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
};

#endif
