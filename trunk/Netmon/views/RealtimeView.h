#ifndef REALTIME_VIEW_H
#define REALTIME_VIEW_H

#include "NetView.h"

class RealtimeView : public NetView
{
protected:
	typedef struct tagRtViewItem
	{
		std::vector<int> rate_tx_1s;
		std::vector<int> rate_tx_10s;
		std::vector<int> rate_tx_60s;

		std::vector<int> rate_rx_1s;
		std::vector<int> rate_rx_10s;
		std::vector<int> rate_rx_60s;

		TCHAR processName[MAX_PATH];

		struct tagRtViewItem()
		{
			rate_tx_1s.reserve(3600 * 10);
			rate_tx_10s.reserve(360 * 10);
			rate_tx_60s.reserve(60 * 10);

			rate_rx_1s.reserve(3600 * 10);
			rate_rx_10s.reserve(360 * 10);
			rate_rx_60s.reserve(60 * 10);

			RtlZeroMemory(processName, sizeof(processName));
		}
	} RtViewItem;

	static enum ZoomFactor
	{
		ZOOM_1S,
		ZOOM_10S,
		ZOOM_60S
	} _zoomFactor;

	static enum SmoothFactor
	{
		SMOOTH_1X,
		SMOOTH_2X,
		SMOOTH_4X
	} _smoothFactor;

	static HDC     _hdcTarget;
	static HDC     _hdcBuf;
	static HBITMAP _hbmpBuf;

	static HFONT   _hOldFont;
	static HFONT   _hEnglishFont;
	static HFONT   _hShellDlgFont;
	static HFONT   _hProcessFont;

	static std::map<int, RtViewItem> _items;

	static int _startTime; // In seconds
	static int _pos;

	static CRITICAL_SECTION _stCS;

protected:
	static void Fill();
	static void DrawGraph();
	static void WINAPI TimerProc(HWND hWnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime);

public:
	virtual void Init();
	virtual void End();
	virtual void InsertPacket(PacketInfoEx *pi);
	virtual void SetProcessUid(int puid, TCHAR *processName);

	virtual LRESULT DlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
};

#endif
