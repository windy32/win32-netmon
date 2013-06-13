#include "stdafx.h"
#include "StatisticsView.h"

#include "../utils/Utils.h"
#include "../utils/ProcessModel.h"
#include "../GdiWidget/GwPieChart.h"
#include "../GdiWidget/GwHistogram.h"
#include "../GdiWidget/GwLogHistogram.h"
#include "../GdiWidget/GwGroupbox.h"
#include "../GdiWidget/GwLabel.h"

#pragma region Members of StatisticsView

// GDI Objects
HDC     StatisticsView::_hdcTarget;
HDC     StatisticsView::_hdcBuf;
HBITMAP StatisticsView::_hbmpBuf;

// Model Object
StatisticsModel *StatisticsView::_model;

#pragma endregion

void StatisticsView::Init(StatisticsModel *model)
{
	_process = PROCESS_ALL;
	_model = model;

	_hdcBuf = 0;
	_hbmpBuf = 0;
}

void StatisticsView::End()
{
	DeleteDC(_hdcBuf);
	DeleteObject(_hbmpBuf);

	_model->SaveDatabase();
}

void StatisticsView::SetProcessUid(int puid)
{
	_process = puid;
	DrawGraph();
}

void StatisticsView::TimerProc(HWND hWnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
{
	_model->UpdateRate();
	DrawGraph();
}

void StatisticsView::DrawGraph()
{
	//  -------------------   -------------------    -------------------    -------------------
	// Summary             | Protocol            |  PacketSize          |  Rate                |
	// |                   | |                   |  |                   |  |                   |
	// |                   | |                   |  |                   |  |                   |
	// |                   | |                   |  |                   |  |                   |
	// |                   | |                   |  |                   |  |                   |
	// |                   | |                   |  |                   |  |                   |
	// |                   | |                   |  |                   |  |                   |
	// |                   | |                   |  |                   |  |                   |
	//  -------------------   -------------------    -------------------    -------------------
	// Rectangle for summary box
	int smr_x      = 12;
	int smr_y      = 8;
	int smr_width  = 160;
	int smr_height = _height - 8 - 10;

	int smrct_x      = 0;
	int smrct_y      = 0;
	int smrct_width  = 0;
	int smrct_height = 0;

	// Rectangle for protocol box
	int ptl_x      = smr_x + smr_width + 16;
	int ptl_y      = 8;
	int ptl_width  = 220 ;
	int ptl_height = _height - 8 - 10;

	// Rectangle for protocol box's content
	int ptlct_x      = 0;
	int ptlct_y      = 0;
	int ptlct_width  = 0;
	int ptlct_height = 0;

	// Rectangle for PacketSize Box
	int psz_x      = ptl_x + ptl_width + 16;
	int psz_y      = 8;
	int psz_width  = 200;
	int psz_height = _height - 8 - 10 ;

	// Rectangle for protocol box's content
	int pszct_x      = 0;
	int pszct_y      = 0;
	int pszct_width  = 0;
	int pszct_height = 0;

	// Rectangle for Rate Box
	int rte_x      = psz_x + psz_width + 16;
	int rte_y      = 8;
	int rte_width  = 200;
	int rte_height = _height - 8 - 10 ;

	// Rectangle for rate box's content
	int rtect_x      = 0;
	int rtect_y      = 0;
	int rtect_width  = 0;
	int rtect_height = 0;

	// Export Model Info
	StatisticsModel::StModelItem item;
	_model->Export(_process, item);

	// Clear Background
	Rectangle(_hdcBuf, -1, -1, _width + 1, _height + 1);

	// Summary --------------------------------------------------------------------------------------------------------
	if (_process == PROCESS_ALL)
	{
		GwGroupbox boxSummay(_hdcBuf, smr_x, smr_y, smr_width, smr_height,
			Language::GetString(IDS_ALL_PROCESS), RGB(0x30, 0x51, 0x9B));
		boxSummay.Paint();
		boxSummay.GetContentArea(&smrct_x, &smrct_y, &smrct_width, &smrct_height);
	}
	else
	{
		TCHAR buf[MAX_PATH];
		ProcessModel::GetProcessName(_process, buf, MAX_PATH);

		GwGroupbox boxSummay(_hdcBuf, smr_x, smr_y, smr_width, smr_height,
			buf, RGB(0x30, 0x51, 0x9B));
		boxSummay.Paint();
		boxSummay.GetContentArea(&smrct_x, &smrct_y, &smrct_width, &smrct_height);
	}

	__int64 avgTxPacketSize = 0;
	__int64 avgRxPacketSize = 0;
	__int64 txPacketCount = 0;
	__int64 rxPacketCount = 0;

	for (int i = 0; i < 1501; i++) // 1 to 1501+ bytes
	{
		avgTxPacketSize += (i + 1) * item.txPacketSize[i];
		avgRxPacketSize += (i + 1) * item.rxPacketSize[i];
		txPacketCount += item.txPacketSize[i];
		rxPacketCount += item.rxPacketSize[i];
	}
	avgTxPacketSize /= txPacketCount;
	avgRxPacketSize /= rxPacketCount;

	__int64 avgTxRate = 0;
	__int64 avgRxRate = 0;
	__int64 txSecondCount = 0;
	__int64 rxSecondCount = 0;

	for (int i = 0; i < 1025; i++) // 0 to 1024+ KB/s
	{
		avgTxRate += i * item.txRate[i];
		avgRxRate += i * item.rxRate[i];
		txSecondCount += item.txRate[i];
		rxSecondCount += item.rxRate[i];
	}
	avgTxRate /= txSecondCount;
	avgRxRate /= rxSecondCount;

	int txIdleRatio = (int)(item.txRate[0] * 1000 / txSecondCount); // 0.0% ~ 100.0%
	int rxIdleRatio = (int)(item.rxRate[0] * 1000 / rxSecondCount);

	TCHAR buf[64];

	_stprintf_s(buf, 64, TEXT("%s: %I64d"), Language::GetString(IDS_STVIEW_AVG_TX_PKT_SIZE), avgTxPacketSize);
	GwLabel lblAverageTxPacketSize(_hdcBuf, smrct_x, smrct_y + 10, smrct_width, smrct_height, buf);

	_stprintf_s(buf, 64, TEXT("%s: %I64d"), Language::GetString(IDS_STVIEW_AVG_RX_PKT_SIZE), avgRxPacketSize);
	GwLabel lblAverageRxPacketSize(_hdcBuf, smrct_x, smrct_y + 30, smrct_width, smrct_height, buf);

	_stprintf_s(buf, 64, TEXT("%s: %d.%d%%"), Language::GetString(IDS_STVIEW_TX_IDLE_RATIO), txIdleRatio / 10, txIdleRatio % 10);
	GwLabel lblTxIdleRatio(_hdcBuf, smrct_x, smrct_y + 50, smrct_width, smrct_height, buf);

	_stprintf_s(buf, 64, TEXT("%s: %d.%d%%"), Language::GetString(IDS_STVIEW_RX_IDLE_RATIO), rxIdleRatio / 10, rxIdleRatio % 10);
	GwLabel lblRxIdleRatio(_hdcBuf, smrct_x, smrct_y + 70, smrct_width, smrct_height, buf);

	_stprintf_s(buf, 64, TEXT("%s: %I64d KB/s"), Language::GetString(IDS_STVIEW_AVG_TX_RATE), avgTxRate);
	GwLabel lblAverageTxRate(_hdcBuf, smrct_x, smrct_y + 90, smrct_width, smrct_height, buf);

	_stprintf_s(buf, 64, TEXT("%s: %I64d KB/s"), Language::GetString(IDS_STVIEW_AVG_RX_RATE), avgRxRate);
	GwLabel lblAverageRxRate(_hdcBuf, smrct_x, smrct_y + 110, smrct_width, smrct_height, buf);

	lblAverageTxPacketSize.Paint();
	lblAverageRxPacketSize.Paint();
	lblTxIdleRatio.Paint();
	lblRxIdleRatio.Paint();
	lblAverageTxRate.Paint();
	lblAverageRxRate.Paint();

	// Protocol -------------------------------------------------------------------------------------------------------
	GwGroupbox boxProtocol(_hdcBuf, ptl_x, ptl_y, ptl_width, ptl_height, 
		Language::GetString(IDS_STVIEW_PROTOCOL), RGB(0x30, 0x51, 0x9B));
	boxProtocol.Paint();
	boxProtocol.GetContentArea(&ptlct_x, &ptlct_y, &ptlct_width, &ptlct_height);

	COLORREF colors[5] = 
	{
		RGB(0xB0, 0xC4, 0xDE),
		RGB(0x9A, 0xCD, 0x32),
		RGB(0xDE, 0xD8, 0x87),
		RGB(0x64, 0x95, 0xED),
		RGB(0xC6, 0xE2, 0xFF)
	};

	const TCHAR *txDescs[5] = 
	{
		Language::GetString(IDS_STVIEW_TX_TCP),
		Language::GetString(IDS_STVIEW_TX_UDP),
		Language::GetString(IDS_STVIEW_TX_ICMP),
		Language::GetString(IDS_STVIEW_TX_IGMP),
		Language::GetString(IDS_STVIEW_TX_OTHER),
	};

	__int64 txValues[5] = 
	{
		item.tx.tcpBytes,
		item.tx.udpBytes,
		item.tx.icmpBytes,
		item.tx.igmpBytes,
		item.tx.otherBytes
	};

	const TCHAR *rxDescs[5] = 
	{
		Language::GetString(IDS_STVIEW_RX_TCP),
		Language::GetString(IDS_STVIEW_RX_UDP),
		Language::GetString(IDS_STVIEW_RX_ICMP),
		Language::GetString(IDS_STVIEW_RX_IGMP),
		Language::GetString(IDS_STVIEW_RX_OTHER),
	};

	__int64 rxValues[5] = 
	{
		item.rx.tcpBytes,
		item.rx.udpBytes,
		item.rx.icmpBytes,
		item.rx.igmpBytes,
		item.rx.otherBytes
	};

	GwPieChart txPieChart(_hdcBuf, ptlct_x, ptlct_y, ptlct_width, (ptlct_height - 4) / 2, txDescs, colors, txValues, 5);
	txPieChart.Paint();

	GwPieChart rxPieChart(_hdcBuf, ptlct_x, ptlct_y + (ptlct_height - 4) / 2 + 4, ptlct_width, ptlct_height / 2, rxDescs, colors, rxValues, 5);
	rxPieChart.Paint();

	// Packet Size ----------------------------------------------------------------------------------------------------
	GwGroupbox boxPacketSize(_hdcBuf, psz_x, psz_y, psz_width, psz_height, 
		Language::GetString(IDS_STVIEW_PACKET_SIZE), RGB(0x30, 0x51, 0x9B));
	boxPacketSize.Paint();
	boxPacketSize.GetContentArea(&pszct_x, &pszct_y, &pszct_width, &pszct_height);

	int pszScales[4] = {1, 100, 600, 1500};

	GwLogHistogram txPszHistogram(_hdcBuf, pszct_x, pszct_y, pszct_width, pszct_height / 2, 
		item.txPacketSize, 1501, pszScales, 4, Language::GetString(IDS_STVIEW_TX), RGB(0xDF, 0x00, 0x24), 4, 4);
	GwLogHistogram rxPszHistogram(_hdcBuf, pszct_x, pszct_y + pszct_height / 2, pszct_width, pszct_height / 2, 
		item.rxPacketSize, 1501, pszScales, 4, Language::GetString(IDS_STVIEW_RX), RGB(0x31, 0x77, 0xC1), 4, 4);

	txPszHistogram.Paint();
	rxPszHistogram.Paint();

	// Rate -----------------------------------------------------------------------------------------------------------
	GwGroupbox boxRate(_hdcBuf, rte_x, rte_y, rte_width, rte_height, 
		Language::GetString(IDS_STVIEW_RATE), RGB(0x30, 0x51, 0x9B));
	boxRate.Paint();
	boxRate.GetContentArea(&rtect_x, &rtect_y, &rtect_width, &rtect_height);

	int rteScales[3] = {10, 150, 1024};

	GwLogHistogram txRteHistogram(_hdcBuf, 
		rtect_x, rtect_y, rtect_width, rtect_height / 2, 
		item.txRate, 1025, rteScales, 3, Language::GetString(IDS_STVIEW_TX), RGB(0xDF, 0x00, 0x24), 4, 4);
	GwLogHistogram rxRteHistogram(_hdcBuf, 
		rtect_x, rtect_y + rtect_height / 2, rtect_width, rtect_height / 2, 
		item.rxRate, 1025, rteScales, 3, Language::GetString(IDS_STVIEW_RX), RGB(0x31, 0x77, 0xC1), 4, 4);

	txRteHistogram.Paint();
	rxRteHistogram.Paint();

	// Write to Screen
	BitBlt(_hdcTarget, 0, 0, _width, _height, _hdcBuf, 0, 0, SRCCOPY);
}

LRESULT StatisticsView::DlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if( uMsg == WM_INITDIALOG )
	{
		// Size Window
		RECT stRect = *(RECT *)lParam;

		_width  = stRect.right - stRect.left;
		_height = stRect.bottom - stRect.top;

		MoveWindow(hWnd, stRect.left, stRect.top, _width, _height, TRUE);

		// Init GDI Objects

		// - Device Context & Bitmap
		_hdcTarget = GetDC(hWnd);

		if( _hdcBuf == 0 )
		{
			_hdcBuf = CreateCompatibleDC(_hdcTarget);
			_hbmpBuf = CreateCompatibleBitmap(_hdcTarget, 2000, 1200);  // Suppose 2000 x 1200 is enough for view

			SelectObject(_hdcBuf, _hbmpBuf);
		}

		// - Clear Background
		Rectangle(_hdcBuf, -1, -1, _width + 1, _height + 1);

		// - Pen
		SelectObject(_hdcBuf, GetStockObject(DC_PEN));

		// - Brush
		SelectObject(_hdcBuf, GetStockObject(DC_BRUSH));

		// - Background Mode
		//SetBkMode(_hdcBuf, TRANSPARENT);

		// Start Timer
		SetTimer(hWnd, 0, 1000, StatisticsView::TimerProc);
	}
	else if( uMsg == WM_CLOSE )
	{
		KillTimer(hWnd, 0);

		ReleaseDC(hWnd, _hdcTarget);
		_hdcTarget = 0;

		DestroyWindow(hWnd);
	}
	else if( uMsg == WM_PAINT )
	{
		PAINTSTRUCT stPS;
		BeginPaint(hWnd, &stPS);

		DrawGraph();

		EndPaint(hWnd, &stPS);
	}
	else if( uMsg == WM_SIZE )
	{
		RECT stRect;

		GetClientRect(hWnd, &stRect);

		_width  = stRect.right - stRect.left;
		_height = stRect.bottom - stRect.top;

		DrawGraph();
	}
	else
	{
		return FALSE;
	}

	return TRUE;
}
