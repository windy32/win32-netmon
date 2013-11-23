#include "stdafx.h"
#include "DetailView.h"

#include "../utils/Utils.h"
#include "../utils/Process.h"
#include "../utils//Language.h"

#include "../res/resource.h"

#pragma region Members of DetailView

std::map<int, DetailView::DtViewItem> DetailView::_items;
HWND DetailView::_hWnd;
HWND DetailView::_hList;
int DetailView::_iLanguageId;
__int64 DetailView::_curPage;
WNDPROC DetailView::_lpOldProcEdit;

CRITICAL_SECTION DetailView::_stCS;

#pragma endregion

// Get Enter from an edit control
LRESULT CALLBACK DetailView::MyProcEdit(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if(uMsg == WM_KEYDOWN && wParam == VK_RETURN )
	{
		if( _items[_process].curPackets > 0 )
		{
			OnGoto();
		}
	}
	else
	{
		return CallWindowProc(_lpOldProcEdit, hWnd, uMsg, wParam, lParam);
	}

	return 0;
}


void DetailView::Init()
{
	_process = PROCESS_ALL;
	_curPage = 0;

	_items[PROCESS_ALL] = DtViewItem();
	_tcscpy_s(_items[PROCESS_ALL].processName, MAX_PATH, TEXT("All Process"));

	InitializeCriticalSection(&_stCS);

	// Read Database
	InitDatabase();
}

void DetailView::End()
{
	DeleteCriticalSection(&_stCS);

	SaveDatabase();
}

void DetailView::InitDatabase()
{
	int puid;
	TCHAR name[256];
	__int64 packetCount;
	TCHAR command[256];

	for(int i = 0; i < Process::GetProcessCount(); i++)
	{
		// Get puid, name and packetCount
		puid = Process::GetProcessUid(i);
		Process::GetProcessName(puid, name, _countof(name));

		_stprintf_s(command, _countof(command), TEXT("Select Count From PacketCount Where ProcessUid = \'%d\';"), puid);

		SQLiteRow row;
		row.InsertType(SQLiteRow::TYPE_INT64); // 0 Count(*)
		if( SQLite::Select(command, &row))
		{
			packetCount = row.GetDataInt64(0);
		}
		else
		{
			packetCount = 0;
		}

		// Insert DtViewItem
		_items[puid] = DtViewItem();
		_tcscpy_s(_items[puid].processName, MAX_PATH, name);
		_items[puid].curPackets = packetCount;

		// Update corresponding PROCESS_ALL item
		_items[PROCESS_ALL].curPackets += packetCount;
	}
}

void DetailView::SaveDatabase()
{
	// Delete all records
	SQLite::Exec(TEXT("Delete From PacketCount;"), true);

	// Insert records
	for(std::map<int, DtViewItem>::iterator it = _items.begin(); it != _items.end(); ++it)
	{
		int puid = it->first;
		__int64 count = it->second.curPackets;

		if( puid == PROCESS_ALL )
		{
			continue;
		}

		// Build Command
		TCHAR command[256];
		_stprintf_s(command, _countof(command), TEXT("Insert Into PacketCount Values(%d, %I64d);"), puid, count);

		// Insert
		SQLite::Exec(command, true);
	}

	// Flush
	SQLite::Flush();
}

void DetailView::InsertPacket(PacketInfoEx *pi)
{
	// Insert an DtViewItem if PUID not Exist
	if( _items.count(pi->puid) == 0 )
	{
		_items[pi->puid] = DtViewItem();
		_tcscpy_s(_items[pi->puid].processName, MAX_PATH, pi->name);
	}

	// Update packet count
	_items[pi->puid].curPackets += 1;
	_items[PROCESS_ALL].curPackets += 1;
}

void DetailView::SetProcessUid(int puid, TCHAR *processName)
{
	_process = puid;
	_curPage = 0;

	UpdateContent(true);
}

void DetailView::TimerProc(HWND hWnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
{
	UpdateContent(false);
}

// ListView Operations
void DetailView::ListViewInsert(int uid, int puid, int dir, int protocol, int size, __int64 time, int port)
{
	// Prepare Columns
	TCHAR szDateTime[MAX_PATH];
	TCHAR szColumn[7][MAX_PATH];

	Language::GetDateTimeString(szDateTime, MAX_PATH, (int)(time >> 32), (int)(time & 0xFFFFFFFF));

	_stprintf_s(szColumn[0], MAX_PATH, TEXT("%d"), uid);
	_stprintf_s(szColumn[1], MAX_PATH, TEXT("%d"), puid);
	_stprintf_s(szColumn[2], MAX_PATH, dir == DIR_UP ? 
		Language::GetString(IDS_DTVIEW_LIST_TX) : 
		Language::GetString(IDS_DTVIEW_LIST_RX));
	_stprintf_s(szColumn[3], MAX_PATH, (protocol == TRA_TCP) ? TEXT("TCP") : 
	                                   (protocol == TRA_UDP) ? TEXT("UDP") : 
	                                   (protocol == TRA_ICMP) ? TEXT("ICMP") : 
	                                   (protocol == TRA_IGMP) ? TEXT("IGMP") : TEXT("OTHER"));
	_stprintf_s(szColumn[4], MAX_PATH, TEXT("%d"), size);
	_stprintf_s(szColumn[5], MAX_PATH, szDateTime);

	if( protocol == TRA_TCP || protocol == TRA_UDP )
	{
		_stprintf_s(szColumn[6], MAX_PATH, TEXT("%d"), port);
	}
	else
	{
		_stprintf_s(szColumn[6], MAX_PATH, TEXT("-"));
	}

	Utils::ListViewAppend(_hList, 7, 
		szColumn[0], szColumn[1], szColumn[2], szColumn[3], szColumn[4], szColumn[5], szColumn[6]);
}

// Resize & Update Content
void DetailView::UpdateSize(HWND hWnd)
{
	// Resize
	RECT stRect;

	GetClientRect(_hWnd, &stRect);

	_width  = stRect.right - stRect.left;
	_height = stRect.bottom - stRect.top;

	MoveWindow(GetDlgItem(hWnd, IDL_DETAIL),   10,           10,           _width - 20, _height - 50, TRUE);
	MoveWindow(GetDlgItem(hWnd, IDB_PAGEUP),   10,           _height - 34, 50,          24,           TRUE);
	MoveWindow(GetDlgItem(hWnd, IDB_PAGEDOWN), 70,           _height - 34, 50,          24,           TRUE);
	MoveWindow(GetDlgItem(hWnd, IDL_STATUS),   130,          _height - 30, 300,         20,           TRUE);
	MoveWindow(GetDlgItem(hWnd, IDB_GOTO),     _width - 150, _height - 34, 80,          24,           TRUE);
	MoveWindow(GetDlgItem(hWnd, IDE_GOTO),     _width - 60,  _height - 34, 50,          24,           TRUE);
}

void DetailView::UpdateContent(bool rebuildList)
{
	EnterCriticalSection(&_stCS);
	
	TCHAR status[256];

	__int64 prevPackets = _items[_process].prevPackets;
	__int64 curPackets  = _items[_process].curPackets;

	const TCHAR *szFormat = Language::GetString(IDS_DTVIEW_PAGE); // Like "%s Page %I64d / %I64d - %I64d"

	if( curPackets == 0 )
	{
		// Status Label
		_stprintf_s(status, 256, szFormat, 
			Language::GetString(IDS_ALL_PROCESS), 
			(__int64)0, (__int64)0, (__int64)0);
		SetDlgItemText(_hWnd, IDL_STATUS, status);

		// Clear list if necessary
		if( prevPackets != 0 )
		{
			Utils::ListViewClear(_hList);
			_items[_process].prevPackets = 0;
		}
	}
	else
	{
		_stprintf_s(status, 256, szFormat, 
			_items[_process].processName, 
			_curPage + 1, (curPackets - 1) / 100 + 1, curPackets);
		SetDlgItemText(_hWnd, IDL_STATUS, status);

		// Button
		EnableWindow(GetDlgItem(_hWnd, IDB_GOTO), TRUE);
		EnableWindow(GetDlgItem(_hWnd, IDB_PAGEUP), (_curPage > 0) ? TRUE : FALSE);
		EnableWindow(GetDlgItem(_hWnd, IDB_PAGEDOWN), (_curPage + 1 < (curPackets - 1) / 100 + 1) ? TRUE : FALSE);

		// ListView
		if( (curPackets > prevPackets && _curPage == prevPackets / 100) // New packets arrived in current page, s
			|| rebuildList )                                            // or force update
		{
			// Select and insert items in current page
			__int64 firstRow = _curPage * 100; // 0-Based

			TCHAR command[256];
			SQLiteRow row;

			if( _process == PROCESS_ALL )
			{
				_stprintf_s(command, _countof(command), 
					TEXT("Select * From Packet Limit 100 Offset %I64d;"), firstRow);
			}
			else
			{
				_stprintf_s(command, _countof(command), 
					TEXT("Select * From Packet Where ProcessUid = %d Limit 100 Offset %I64d;"), _process, firstRow);
			}

			row.InsertType(SQLiteRow::TYPE_INT32); // 0 UID
			row.InsertType(SQLiteRow::TYPE_INT32); // 1 PActivityUid
			row.InsertType(SQLiteRow::TYPE_INT32); // 2 ProcessUid
			row.InsertType(SQLiteRow::TYPE_INT32); // 3 AdapterUid
			row.InsertType(SQLiteRow::TYPE_INT32); // 4 Direction
			row.InsertType(SQLiteRow::TYPE_INT32); // 5 NetProtocol
			row.InsertType(SQLiteRow::TYPE_INT32); // 6 TraProtocol
			row.InsertType(SQLiteRow::TYPE_INT32); // 7 Size
			row.InsertType(SQLiteRow::TYPE_INT64); // 8 Time
			row.InsertType(SQLiteRow::TYPE_INT32); // 9 Port

			// Delete all items
			Utils::ListViewClear(_hList);

			SQLite::Select(command, &row, UpdateContentCallback);

			// Update prevPackets
			_items[_process].prevPackets = curPackets;
		}
	}

	LeaveCriticalSection(&_stCS);
}

void DetailView::UpdateContentCallback(SQLiteRow *row)
{
	int uid      = row->GetDataInt32(0);
	int puid     = row->GetDataInt32(2);
	int dir      = row->GetDataInt32(4);
	int protocol = row->GetDataInt32(6);
	int size     = row->GetDataInt32(7);
	__int64 time = row->GetDataInt64(8);
	int port     = row->GetDataInt32(9);

	ListViewInsert(uid, puid, dir, protocol, size, time, port);
}

// Flip Page
void DetailView::OnPageUp()
{
	_curPage -= 1;
	UpdateContent(true);

	__int64 curPackets  = _items[_process].curPackets;

	EnableWindow(GetDlgItem(_hWnd, IDB_PAGEUP), (_curPage > 0) ? TRUE : FALSE);
	EnableWindow(GetDlgItem(_hWnd, IDB_PAGEDOWN), (_curPage + 1 < (curPackets - 1) / 100 + 1) ? TRUE : FALSE);
}

void DetailView::OnPageDown()
{
	_curPage += 1;
	UpdateContent(true);

	__int64 curPackets  = _items[_process].curPackets;

	EnableWindow(GetDlgItem(_hWnd, IDB_PAGEUP), (_curPage > 0) ? TRUE : FALSE);
	EnableWindow(GetDlgItem(_hWnd, IDB_PAGEDOWN), (_curPage + 1 < (curPackets - 1) / 100 + 1) ? TRUE : FALSE);
}

void DetailView::OnGoto()
{
	BOOL translated;
	int page = GetDlgItemInt(_hWnd, IDE_GOTO, &translated, TRUE);
	__int64 curPackets  = _items[_process].curPackets;

	if( !translated ) 
	{
		MessageBox(_hWnd, TEXT("Page number incorrect!"), TEXT("Error"), MB_ICONWARNING | MB_OK);
	}
	else if( page <= 0 || page > (curPackets - 1) / 100 + 1 )
	{
		MessageBox(_hWnd, TEXT("Page number out of range!"), TEXT("Error"), MB_ICONWARNING | MB_OK);
	}
	else
	{
		_curPage = page - 1;
		UpdateContent(true);

		EnableWindow(GetDlgItem(_hWnd, IDB_PAGEUP), (_curPage > 0) ? TRUE : FALSE);
		EnableWindow(GetDlgItem(_hWnd, IDB_PAGEDOWN), (_curPage + 1 < (curPackets - 1) / 100 + 1) ? TRUE : FALSE);
	}
}

void DetailView::SwitchLanguage(HWND hWnd)
{
	// List view column header
	Utils::ListViewSetColumnText(GetDlgItem(hWnd, IDL_DETAIL), 7, 
		Language::GetString(IDS_DTVIEW_LIST_UID),
		Language::GetString(IDS_DTVIEW_LIST_PROCESS),
		Language::GetString(IDS_DTVIEW_LIST_DIR),
		Language::GetString(IDS_DTVIEW_LIST_PROTOCOL),
		Language::GetString(IDS_DTVIEW_LIST_SIZE),
		Language::GetString(IDS_DTVIEW_LIST_TIME),
		Language::GetString(IDS_DTVIEW_LIST_PORT));

	// List view content & label
	UpdateContent(true);

	// Button
	SetDlgItemText(hWnd, IDB_GOTO, Language::GetString(IDS_DTVIEW_GOTO));
}

// Dialog Procedure
LRESULT DetailView::DlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if( uMsg == WM_INITDIALOG )
	{
		_hWnd = hWnd;
		_hList = GetDlgItem(hWnd, IDL_DETAIL);

		_lpOldProcEdit = (WNDPROC)SetWindowLong(GetDlgItem(_hWnd, IDE_GOTO), GWL_WNDPROC, (LONG)MyProcEdit);

		// Size Window
		RECT stRect = *(RECT *)lParam;

		_width  = stRect.right - stRect.left;
		_height = stRect.bottom - stRect.top;

		MoveWindow(hWnd, stRect.left, stRect.top, _width, _height, TRUE);

		// Save current language id
		_iLanguageId = Language::GetLangId();

		// Init button
		SetDlgItemText(hWnd, IDB_GOTO, Language::GetString(IDS_DTVIEW_GOTO));

		// Init ListView
		Utils::ListViewInit(_hList, FALSE, 7, 
			Language::GetString(IDS_DTVIEW_LIST_UID),
			Language::GetString(IDS_DTVIEW_LIST_PROCESS),
			Language::GetString(IDS_DTVIEW_LIST_DIR),
			Language::GetString(IDS_DTVIEW_LIST_PROTOCOL),
			Language::GetString(IDS_DTVIEW_LIST_SIZE),
			Language::GetString(IDS_DTVIEW_LIST_TIME),
			Language::GetString(IDS_DTVIEW_LIST_PORT),
			60, 70, 50, 80, 60, 220, 60);

		UpdateContent(true);

		// Start Timer
		SetTimer(hWnd, 0, 1000, DetailView::TimerProc);
	}
	else if( uMsg == WM_PAINT )
	{
		if( Language::GetLangId() != _iLanguageId )
		{
			SwitchLanguage(hWnd);
			_iLanguageId = Language::GetLangId();
		}
		return FALSE;
	}
	else if( uMsg == WM_COMMAND )
	{
		if( wParam == IDB_PAGEUP )
		{
			OnPageUp();
		}
		else if( wParam == IDB_PAGEDOWN )
		{
			OnPageDown();
		}
		else if( wParam == IDB_GOTO )
		{
			OnGoto();
		}
	}
	else if(uMsg == WM_CTLCOLORSTATIC || uMsg == WM_CTLCOLORBTN )
	{
		return RGB(255, 255, 255);
	}
	else if(uMsg == WM_CTLCOLORDLG )
	{
		return (INT_PTR)GetStockObject(WHITE_BRUSH);
	}
	else if( uMsg == WM_CLOSE )
	{
		KillTimer(hWnd, 0);
		DestroyWindow(hWnd);
		_hWnd = 0;
	}
	else if( uMsg == WM_SIZE )
	{
		UpdateSize(hWnd);
	}
	else
	{
		return FALSE;
	}

	return TRUE;
}

__int64 DetailView::GetPacketCount()
{
	return _items[PROCESS_ALL].curPackets;
}

void DetailView::OnAllPacketsDeleted()
{
	for(std::map<int, DtViewItem>::iterator it = _items.begin(); it != _items.end(); ++it)
	{
		it->second.curPackets = 0;
	}
	_curPage = 0;

	if( _hWnd != 0 )
	{
		UpdateContent(true);
	}
}