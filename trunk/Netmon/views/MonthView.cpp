#include "stdafx.h"
#include "MonthView.h"

#include "../utils/Utils.h"
#include "../utils/Process.h"

#include "../res/resource.h"

#pragma region Members of MonthView

int MonthView::MtViewItem::firstMonth = -1;

std::map<int, MonthView::MtViewItem> MonthView::_items;
int MonthView::_curMonth = -1;

HFONT MonthView::_hFontDays;
HFONT MonthView::_hFontDesc;
HPEN  MonthView::_hPenVertical;

HDC     MonthView::_hdcTarget;
HDC     MonthView::_hdcBuf;
HBITMAP MonthView::_hbmpBuf;

HDC     MonthView::_hdcPage;
HBITMAP MonthView::_hbmpPageUpLight;
HBITMAP MonthView::_hbmpPageUpDark;
HBITMAP MonthView::_hbmpPageDownLight;
HBITMAP MonthView::_hbmpPageDownDark;

MonthView * MonthView::_this;

CRITICAL_SECTION MonthView::_stCS;

#pragma endregion

void MonthView::Init()
{
	_process = PROCESS_ALL;

	_items[PROCESS_ALL] = MtViewItem();
	_tcscpy_s(_items[PROCESS_ALL].processName, MAX_PATH, TEXT("All Process"));

	_hdcBuf = 0;
	_hbmpBuf = 0;

	_hbmpPageUpLight   = LoadBitmap(GetModuleHandle(0), MAKEINTRESOURCE(IDB_PAGEUP_LT));
	_hbmpPageUpDark    = LoadBitmap(GetModuleHandle(0), MAKEINTRESOURCE(IDB_PAGEUP_DK));
	_hbmpPageDownLight = LoadBitmap(GetModuleHandle(0), MAKEINTRESOURCE(IDB_PAGEDN_LT));
	_hbmpPageDownDark  = LoadBitmap(GetModuleHandle(0), MAKEINTRESOURCE(IDB_PAGEDN_DK));

	_this = this;

	InitializeCriticalSection(&_stCS);

	// Read Database
	InitDatabase();
}

void MonthView::End()
{
	DeleteDC(_hdcBuf);
	DeleteObject(_hbmpBuf);

	DeleteCriticalSection(&_stCS);

	SaveDatabase();
}

void MonthView::SaveDatabase()
{
	// Delete all records
	SQLite::Exec(TEXT("Delete From Traffic;"), true);

	// Insert records
	for(std::map<int, MtViewItem>::iterator it = _items.begin(); it != _items.end(); ++it) // Loop of Process
	{
		int puid = it->first;

		if( puid == PROCESS_ALL )
		{
			continue;
		}

		for(int i = 0; i < (int)it->second.months.size(); i++) // Loop of Month Array
		{
			int exMonth = MtViewItem::firstMonth + i;
			int numDays = Utils::GetNumDays(exMonth);

			for(int j = 0; j < numDays; j++) // Loop of Month
			{
				int date = (exMonth << 16) + (j + 1); // j + 1: [1, 31]

				// Build Command
				TCHAR command[256];

				_stprintf_s(command, _countof(command), TEXT("Insert Into Traffic Values(%d, %d, %I64d, %I64d, 0, 0);"), 
					puid, date, it->second.months[i].dayTx[j], it->second.months[i].dayRx[j]);

				// Insert
				SQLite::Exec(command, true);
			}
		}
	}

	// Flush
	SQLite::Flush();
}

void MonthView::InitDatabase()
{
	static TCHAR command[256];

	// Build Command
	_stprintf_s(command, _countof(command), TEXT("Select * From Traffic;"));

	// Build SQLiteRow Object
	SQLiteRow row;

	row.InsertType(SQLiteRow::TYPE_INT32); // 0 ProcessUid
	row.InsertType(SQLiteRow::TYPE_INT32); // 1 Date
	row.InsertType(SQLiteRow::TYPE_INT64); // 2 TxBytes
	row.InsertType(SQLiteRow::TYPE_INT64); // 3 RxBytes
	row.InsertType(SQLiteRow::TYPE_INT32); // 4 TxPackets
	row.InsertType(SQLiteRow::TYPE_INT32); // 5 RxPackets

	// Select
	SQLite::Select(command, &row, InitDatabaseCallback);

	// Set current month as first month when no data is available
	if( MtViewItem::firstMonth == -1 )
	{
		MtViewItem::firstMonth = Utils::GetExMonth();
	}
	_curMonth = MtViewItem::firstMonth;

	// Fill processName
	for(std::map<int, MtViewItem>::iterator it = _items.begin(); it != _items.end(); ++it)
	{
		Process::GetProcessName(it->first, it->second.processName, MAX_PATH);
	}
}

void MonthView::InitDatabaseCallback(SQLiteRow *row)
{
	int puid        = row->GetDataInt32(0);
	int date        = row->GetDataInt32(1); // Higher 16 bit for exMonth (Jan 1970 = 0), lower 16 bit for mday
	__int64 txBytes = row->GetDataInt64(2);
	__int64 rxBytes = row->GetDataInt64(3);

	int mDay = Utils::GetMdayByDate(date);
	int exMonth = Utils::GetExMonthByDate(date);

	// Insert an MtViewItem if PUID not Exist
	if( _items.count(puid) == 0 )
	{
		_items[puid] = MtViewItem();
		MtViewItem::firstMonth = Utils::GetExMonthByDate(date);
	}

	// Fill Vectors
	Fill();

	while( exMonth - MtViewItem::firstMonth > (int)_items[puid].months.size() - 1)
	{
		_items[puid].months.push_back(MonthItem());
	}

	while( exMonth - MtViewItem::firstMonth > (int)_items[PROCESS_ALL].months.size() - 1)
	{
		_items[PROCESS_ALL].months.push_back(MonthItem());
	}

	// Update Traffic
	MonthItem &mItem = _items[puid].months[exMonth - MtViewItem::firstMonth];
	MonthItem &mItemAll = _items[PROCESS_ALL].months[exMonth - MtViewItem::firstMonth];

	mItem.dayTx[mDay - 1] = txBytes;
	mItem.dayRx[mDay - 1] = rxBytes;
	mItem.sumTx += txBytes;
	mItem.sumRx += rxBytes;

	mItemAll.dayTx[mDay - 1] += txBytes;
	mItemAll.dayRx[mDay - 1] += rxBytes;
	mItemAll.sumTx += txBytes;
	mItemAll.sumRx += rxBytes;
}

void MonthView::InsertPacket(PacketInfoEx *pi)
{
	// Insert an MtViewItem if PUID not Exist
	if( _items.count(pi->puid) == 0 )
	{
		_items[pi->puid] = MtViewItem();
		_tcscpy_s(_items[pi->puid].processName, MAX_PATH, pi->name);
	}

	// Fill
	Fill();

	// Update Traffic
	int mDay = Utils::GetDay((time_t)pi->time_s);

	EnterCriticalSection(&_stCS);

	MonthItem &mItem = _items[pi->puid].months[Utils::GetExMonth() - MtViewItem::firstMonth];
	MonthItem &mItemAll = _items[PROCESS_ALL].months[Utils::GetExMonth() - MtViewItem::firstMonth];

	if( pi->dir == DIR_UP )
	{
		mItem.dayTx[mDay - 1] += pi->size;
		mItem.sumTx += pi->size;

		mItemAll.dayTx[mDay - 1] += pi->size;
		mItemAll.sumTx += pi->size;
	}
	else if( pi->dir == DIR_DOWN )
	{
		mItem.dayRx[mDay - 1] += pi->size;
		mItem.sumRx += pi->size;

		mItemAll.dayRx[mDay - 1] += pi->size;
		mItemAll.sumRx += pi->size;
	}

	LeaveCriticalSection(&_stCS);
}

void MonthView::SetProcessUid(int puid, TCHAR *processName)
{
	_process = puid;
	Fill();

	EnterCriticalSection(&_stCS);

	if( _curMonth > _items[puid].firstMonth + (int)_items[puid].months.size() - 1 )
	{
		_curMonth = _items[puid].firstMonth;
	}

	LeaveCriticalSection(&_stCS);

	DrawGraph();
}
void MonthView::TimerProc(HWND hWnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
{
	// Start Painting
	DrawGraph();
}

void MonthView::Fill()
{
	// Calc the desired length of the vectors
	int exMonth = Utils::GetExMonth();
	int length = exMonth - MtViewItem::firstMonth + 1;

	EnterCriticalSection(&_stCS);

	// Fill vectors
	for(std::map<int, MtViewItem>::iterator it = _items.begin(); it != _items.end(); ++it)
	{
		while( (int)it->second.months.size() < length)
		{
			it->second.months.push_back(MonthItem());
		}
	}

	LeaveCriticalSection(&_stCS);
}

void MonthView::DrawGraph()
{
	// Fill Vectors
	Fill();

	EnterCriticalSection(&_stCS);

	MtViewItem item = _items[_process];
	MonthItem mItem = item.months[_curMonth - MtViewItem::firstMonth];

	LeaveCriticalSection(&_stCS);

	int numDays = Utils::GetNumDays(_curMonth);

	// Rectangle for Graph
	int x1 = 8;
	int y1 = 19;
	int x2 = _width - 48;
	int y2 = _height - 26;

	int colWidth = (_width - 8 - 48) / numDays;

	x2 = x1 + colWidth * numDays;

	// Calc the Maximum Amount of Traffic in a Day
	__int64 maxTraffic = 0;

	for(int i = 0; i < numDays; i++)
	{
		if( mItem.dayTx[i] > maxTraffic )
		{
			maxTraffic = mItem.dayTx[i];
		}

		if( mItem.dayRx[i] > maxTraffic )
		{
			maxTraffic = mItem.dayRx[i];
		}
	}

	maxTraffic /= (1024 * 1024); // Unit is now MB

	// Deside Scale
	//    1. (    0 MB,   100 MB]   100,    80,    60,    40,    20,    0
	//    2. (  100 MB,   200 MB]   200,   150,   100,    50,     0
	//    3. (  200 MB,   500 MB]   500,   400,   300,   200,   100,    0
	//    4. (  500 MB,  1000 MB]  1000,   800,   600,   400,   200,    0
	//    5. ( 1000 MB,  2000 MB]  2000,  1500,  1000,   500,     0
	//    6. ( 2000 MB,  5000 MB]  5000,  4000,  3000,  2000,  1000,    0
	//    7. ( 5000 MB, 10000 MB] 10000,  8000,  6000,  4000,  2000,    0
	//    8. (10000 MB, 20000 MB] 20000, 15000, 10000,  5000,     0,
	//    9. (20000 MB, 50000 MB] 50000, 40000, 30000, 20000, 10000,    0
	//    ( ...More traffic are not currently supported )
	const int traffic[9] = {100, 200, 500, 1000, 2000, 5000, 10000, 20000, 50000};
	const int segments[9] = {5, 4, 5, 5, 4, 5, 5, 4, 5};

	int numSegment = 0;
	int scaleTraffic;

	for(int i = 0; i < 9; i++)
	{
		if( maxTraffic <= traffic[i] )
		{
			scaleTraffic = traffic[i];
			numSegment = segments[i];
			break;
		}
	}

	if( numSegment == 0 ) // More than 50000 MB
	{
		scaleTraffic = traffic[9 - 1];
		numSegment = segments[9 - 1];
	}

	// Clear Background
	Rectangle(_hdcBuf, -1, -1, _width + 1, _height + 1);

	// Bg for Days
	SelectObject(_hdcBuf, GetStockObject(NULL_PEN));
	SetDCBrushColor(_hdcBuf, RGB(0xF0, 0xF0, 0xF0));

	Rectangle(_hdcBuf, x1, y1 - 12, x2, y1 + 2);

	// Box for Graph
	SelectObject(_hdcBuf, GetStockObject(DC_PEN));
	SelectObject(_hdcBuf, GetStockObject(NULL_BRUSH));

	SetDCPenColor(_hdcBuf, RGB(0x80, 0x80, 0x80));
	Rectangle(_hdcBuf, x1, y1 - 12, x2, y2 + 1);

	SetDCPenColor(_hdcBuf, RGB(0xBB, 0xBB, 0xBB));
	Rectangle(_hdcBuf, x1, y1 - 12, x2, y2);

	SelectObject(_hdcBuf, GetStockObject(DC_BRUSH));

	// Draw Vertical Lines
	SelectObject(_hdcBuf, _hPenVertical);

	for(int i = 1; i < numDays; i++)
	{
		MoveToEx(_hdcBuf, x1 + i * colWidth, y1 + 1 - 12, 0);
		LineTo(_hdcBuf, x1 + i * colWidth, y2 - 1);
	}

	// Draw Horizontal Lines
	SelectObject(_hdcBuf, GetStockObject(DC_PEN));
	SetDCPenColor(_hdcBuf, RGB(0xBB, 0xBB, 0xBB));

	if( numSegment == 4 )
	{
		for(int i = 1; i < 5 - 1; i++)
		{
			MoveToEx(_hdcBuf, x1 + 1, y1 + (y2 - y1) * i / 4, 0);
			LineTo(_hdcBuf, x2 - 1, y1 + (y2 - y1) * i / 4);
		}
	}
	else if( numSegment == 5 )
	{
		for(int i = 1; i < 6 - 1; i++)
		{
			MoveToEx(_hdcBuf, x1 + 1, y1 + (y2 - y1) * i / 5, 0);
			LineTo(_hdcBuf, x2 - 1, y1 + (y2 - y1) * i / 5);
		}
	}

	SetDCPenColor(_hdcBuf, RGB(0x00, 0x00, 0x00));

	// Draw Days
	SelectObject(_hdcBuf, _hFontDays);
	SetTextAlign(_hdcBuf, TA_CENTER);

	for(int i = 0; i < numDays; i++)
	{
		TCHAR szDay[32];
		_stprintf_s(szDay, _countof(szDay), TEXT("%d"), i + 1);

		// Set Color, Sundays are Red
		if( Utils::GetWeekDay(item.firstMonth, i + 1) == 0 )
		{
			SetTextColor(_hdcBuf, RGB(0xDF, 0x00, 0x24));
		}
		else
		{
			SetTextColor(_hdcBuf, RGB(0x77, 0x77, 0x77));
		}

		TextOut(_hdcBuf, x1 + colWidth / 2 + i * colWidth, y1 + 2 - 12, szDay, _tcslen(szDay));
	}

	// Draw Scale
	TCHAR yAxisText[16];
	SetTextAlign(_hdcBuf, TA_LEFT);
	SetTextColor(_hdcBuf, RGB(0x00, 0x00, 0x00));

	if( numSegment == 4 )
	{
		for(int i = 0; i < 5 - 1; i++)
		{
			_stprintf_s(yAxisText, _countof(yAxisText), TEXT("%d MB"), scaleTraffic * (4 - i) / 4);
			TextOut(_hdcBuf, x2 + 3, y1 + (y2 - y1) * i / 4 - 5 , yAxisText, _tcslen(yAxisText));
		}
	}
	else if( numSegment == 5 )
	{
		for(int i = 0; i < 6 - 1; i++)
		{
			_stprintf_s(yAxisText, _countof(yAxisText), TEXT("%d MB"), scaleTraffic * (5 - i) / 5);
			TextOut(_hdcBuf, x2 + 3, y1 + (y2 - y1) * i / 5 - 5, yAxisText, _tcslen(yAxisText));
		}
	}

	// Draw Traffic

	// - Rx
	SetDCBrushColor(_hdcBuf, RGB(0x22, 0x8B, 0x22));

	for(int i = 0; i < numDays; i++)
	{
		int rxTraffic = (int)(mItem.dayRx[i] >> 20);
		int yPos = (y2 - y1) * rxTraffic / scaleTraffic;

		Rectangle(_hdcBuf, x1 + 4 + colWidth * i, y2 - yPos - 1, x1 + 4 + colWidth * i + (colWidth - 8) / 2, y2);
	}

	// - Tx
	SetDCBrushColor(_hdcBuf, RGB(0xDF, 0x00, 0x24));

	for(int i = 0; i < numDays; i++)
	{
		int txTraffic = (int)(mItem.dayTx[i] >> 20);
		int yPos = (y2 - y1) * txTraffic / scaleTraffic;

		Rectangle(_hdcBuf, x1 + 4 + colWidth * i + (colWidth - 8) / 2, y2 - yPos - 1, x1 + colWidth - 4 + colWidth * i, y2);
	}

	SetDCBrushColor(_hdcBuf, RGB(0xFF, 0xFF, 0xfF));

	// Process Name & Statistics
	SelectObject(_hdcBuf, _hFontDesc);
	SetTextAlign(_hdcBuf, TA_LEFT);
	SetTextColor(_hdcBuf, RGB(0x00, 0x00, 0x00));

	TCHAR szText[256];
	TCHAR szYearMonth[256];
	Language::GetYearMonthString(szYearMonth, 256, _curMonth / 12 + 1970, _curMonth % 12);

	if( _process == -1 )
	{
		if (mItem.sumRx < 1024 * 1024 && mItem.sumTx < 1024 * 1024)
		{
			const TCHAR *szFormat = Language::GetString(IDS_MTVIEW_TEXT_KB); // Like "%s - %s (Incoming: %d KB / Outgoing: %d KB)"
			_stprintf_s(szText, _countof(szText), szFormat, 
				Language::GetString(IDS_ALL_PROCESS), szYearMonth, (int)(mItem.sumRx >> 10), (int)(mItem.sumTx >> 10));
		}
		else
		{
			const TCHAR *szFormat = Language::GetString(IDS_MTVIEW_TEXT_MB); // Like "%s - %s (Incoming: %d MB / Outgoing: %d MB)"
			_stprintf_s(szText, _countof(szText), szFormat, 
				Language::GetString(IDS_ALL_PROCESS), szYearMonth, (int)(mItem.sumRx >> 20), (int)(mItem.sumTx >> 20));
		}
	}
	else
	{
		if(mItem.sumRx < 1024 * 1024 && mItem.sumTx < 1024 * 1024)
		{
			const TCHAR *szFormat = Language::GetString(IDS_MTVIEW_TEXT_KB); // Like "%s - %s (Incoming: %d KB / Outgoing: %d KB)"
			_stprintf_s(szText, _countof(szText), szFormat, 
				item.processName, szYearMonth, (int)(mItem.sumRx >> 10), (int)(mItem.sumTx >> 10));
		}
		else
		{
			const TCHAR *szFormat = Language::GetString(IDS_MTVIEW_TEXT_MB); // Like "%s - %s (Incoming: %d MB / Outgoing: %d MB)"
			_stprintf_s(szText, _countof(szText), szFormat, 
				item.processName, szYearMonth, (int)(mItem.sumRx >> 20), (int)(mItem.sumTx >> 20));
		}
	}
	TextOut(_hdcBuf, x1 + 1, y2 + 2, szText, _tcslen(szText));

	// Draw PageUp / PageDown Icon
	if( _curMonth == _items[_process].firstMonth ) // No previous month
	{
		SelectObject(_hdcPage, _hbmpPageUpLight);
	}
	else
	{
		SelectObject(_hdcPage, _hbmpPageUpDark);
	}

	BitBlt(_hdcBuf, x2 - 21, y2 + 4, 7, 7, _hdcPage, 0, 0, SRCCOPY);

	if( _curMonth == _items[_process].firstMonth + _items[_process].months.size() - 1 ) // No next month
	{
		SelectObject(_hdcPage, _hbmpPageDownLight);
	}
	else
	{
		SelectObject(_hdcPage, _hbmpPageDownDark);
	}

	BitBlt(_hdcBuf, x2 - 9, y2 + 4, 7, 7, _hdcPage, 0, 0, SRCCOPY);

	// Wriet to Screen
	BitBlt(_hdcTarget, 0, 0, _width, _height, _hdcBuf, 0, 0, SRCCOPY);
}

LRESULT MonthView::DlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
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

			_hdcPage = CreateCompatibleDC(_hdcTarget);
		}

		// - Clear Background
		Rectangle(_hdcBuf, -1, -1, _width + 1, _height + 1);

		// - Font
		_hFontDays = Utils::MyCreateFont(TEXT("Arial"), 12, 5, false);
		_hFontDesc = Utils::MyCreateFont(TEXT("MS Shell Dlg 2"), 14, 0, true);

		// - Pen
		_hPenVertical = CreatePen(PS_DOT, 0, RGB(0xC4, 0xC4, 0xC4));

		SelectObject(_hdcBuf, GetStockObject(DC_PEN));

		// - Brush
		SelectObject(_hdcBuf, GetStockObject(DC_BRUSH));

		// - Background Mode
		SetBkMode(_hdcBuf, TRANSPARENT);

		// Start Timer
		SetTimer(hWnd, 0, 1000, MonthView::TimerProc);
	}
	else if( uMsg == WM_CLOSE )
	{
		KillTimer(hWnd, 0);

		ReleaseDC(hWnd, _hdcTarget);
		_hdcTarget = 0;

		DestroyWindow(hWnd);
	}
	else if( uMsg == WM_LBUTTONDOWN )
	{
		// Rectangle for Graph
		int numDays = Utils::GetNumDays(_curMonth);

		int x1 = 8;
		int y1 = 19;
		int x2 = _width - 48;
		int y2 = _height - 26;

		int colWidth = (_width - 8 - 48) / numDays;
		x2 = x1 + colWidth * numDays;

		// Page State
		int xPos = GET_X_LPARAM(lParam); 
		int yPos = GET_Y_LPARAM(lParam);

		if( xPos >= x2 - 21 && xPos <= x2 - 21 + 7 &&
			yPos >= y2 + 4  && yPos <= y2 + 4  + 7 )
		{
			if( _curMonth > _items[_process].firstMonth ) // Previous month
			{
				_curMonth -= 1;
				DrawGraph();
			}
		}
		else if( xPos >= x2 - 9 && xPos <= x2 - 9 + 7 &&
		         yPos >= y2 + 4 && yPos <= y2 + 4 + 7 )
		{
			if( _curMonth < _items[_process].firstMonth + (int)_items[_process].months.size() - 1 ) // Next month
			{
				_curMonth += 1;
				DrawGraph();
			}
		}
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
