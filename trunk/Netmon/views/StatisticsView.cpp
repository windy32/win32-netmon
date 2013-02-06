#include "stdafx.h"
#include "StatisticsView.h"

#include "../utils/Utils.h"
#include "../utils/Process.h"
#include "../GdiWidget/GwPieChart.h"
#include "../GdiWidget/GwHistogram.h"
#include "../GdiWidget/GwLogHistogram.h"
#include "../GdiWidget/GwGroupbox.h"

#pragma region Members of StatisticsView

std::map<int, StatisticsView::StViewItem> StatisticsView::_items;

HDC     StatisticsView::_hdcTarget;
HDC     StatisticsView::_hdcBuf;
HBITMAP StatisticsView::_hbmpBuf;

StatisticsView *StatisticsView::_this;
CRITICAL_SECTION StatisticsView::_stCS;

#pragma endregion

void StatisticsView::Init()
{
	_process = PROCESS_ALL;

	_items[PROCESS_ALL] = StViewItem();
	_tcscpy_s(_items[PROCESS_ALL].processName, MAX_PATH, TEXT("All Process"));

	_hdcBuf = 0;
	_hbmpBuf = 0;

	_this = this;

	InitializeCriticalSection(&_stCS);

	InitDatabase();
}

void StatisticsView::End()
{
	DeleteDC(_hdcBuf);
	DeleteObject(_hbmpBuf);

	DeleteCriticalSection(&_stCS);

	SaveDatabase();
}

void StatisticsView::SaveDatabase()
{
	TCHAR command[256];

	// Delete all records - Protocol
	SQLite::Exec(TEXT("Delete From Protocol;"), true);

	// Insert records - Protocol
	for(std::map<int, StViewItem>::iterator it = _items.begin(); it != _items.end(); ++it) // Loop of Process
	{
		int puid = it->first;

		if( puid == PROCESS_ALL )
		{
			continue;
		}

		// TCP
		_stprintf_s(command, _countof(command), TEXT("Insert Into Protocol Values(%d, %d, %I64d, %I64d, %I64d, %I64d);"), puid, TRA_TCP, 
			it->second.tx.tcpBytes, 
			it->second.rx.tcpBytes, 
			it->second.tx.tcpPackets, 
			it->second.rx.tcpPackets);
		SQLite::Exec(command, true);

		// UDP
		_stprintf_s(command, _countof(command), TEXT("Insert Into Protocol Values(%d, %d, %I64d, %I64d, %I64d, %I64d);"), puid, TRA_UDP, 
			it->second.tx.udpBytes, 
			it->second.rx.udpBytes, 
			it->second.tx.udpPackets, 
			it->second.rx.udpPackets);
		SQLite::Exec(command, true);

		// ICMP
		_stprintf_s(command, _countof(command), TEXT("Insert Into Protocol Values(%d, %d, %I64d, %I64d, %I64d, %I64d);"), puid, TRA_ICMP, 
			it->second.tx.icmpBytes, 
			it->second.rx.icmpBytes, 
			it->second.tx.icmpPackets, 
			it->second.rx.icmpPackets);
		SQLite::Exec(command, true);

		// IGMP
		_stprintf_s(command, _countof(command), TEXT("Insert Into Protocol Values(%d, %d, %I64d, %I64d, %I64d, %I64d);"), puid, TRA_IGMP, 
			it->second.tx.igmpBytes, 
			it->second.rx.igmpBytes, 
			it->second.tx.igmpPackets, 
			it->second.rx.igmpPackets);
		SQLite::Exec(command, true);

		// OTHER
		_stprintf_s(command, _countof(command), TEXT("Insert Into Protocol Values(%d, %d, %I64d, %I64d, %I64d, %I64d);"), puid, TRA_OTHER, 
			it->second.tx.otherBytes, 
			it->second.rx.otherBytes, 
			it->second.tx.otherPackets, 
			it->second.rx.otherPackets);
		SQLite::Exec(command, true);
	}

	// Insert or update records - PacketSize
	for(std::map<int, StViewItem>::iterator it = _items.begin(); it != _items.end(); ++it) // Loop of Process
	{
		int puid = it->first;

		if( puid == PROCESS_ALL )
		{
			continue;
		}

		if( it->second.newItem == true ) // Insert
		{
			for(int i = 0; i < 1501; i++)
			{
				_stprintf_s(command, _countof(command), TEXT("Insert Into PacketSize Values(%d, %d, 0, 0, %I64d, %I64d);"), 
					puid, i, it->second.txPacketSize[i], it->second.rxPacketSize[i]);

				SQLite::Exec(command, true);
			}
		}
		else // Update
		{
			for(int i = 0; i < 1501; i++)
			{
				if( it->second.txPacketSize[i] > it->second.txPrevPacketSize[i] ||
					it->second.rxPacketSize[i] > it->second.rxPrevPacketSize[i] )
				{
					_stprintf_s(command, _countof(command), 
						TEXT("Update PacketSize Set TxPackets = %I64d, RxPackets = %I64d ")
						TEXT("Where ProcessUid = %d And PacketSize = %d;"), 
						it->second.txPacketSize[i], it->second.rxPacketSize[i], puid, i);

					SQLite::Exec(command, true);
				}
			}
		}
	}

	// Insert or update records - Rate
	for(std::map<int, StViewItem>::iterator it = _items.begin(); it != _items.end(); ++it) // Loop of Process
	{
		int puid = it->first;

		if( puid == PROCESS_ALL )
		{
			continue;
		}

		if( it->second.newItem == true ) // Insert
		{
			for(int i = 0; i < 1025; i++)
			{
				_stprintf_s(command, _countof(command), TEXT("Insert Into Rate Values(%d, %d, %I64d, %I64d);"), 
					puid, i, it->second.txRate[i], it->second.rxRate[i]);

				SQLite::Exec(command, true);
			}
		}
		else // Update
		{
			for(int i = 0; i < 1025; i++)
			{
				if( it->second.txRate[i] > it->second.txPrevRate[i] ||
					it->second.rxRate[i] > it->second.rxPrevRate[i] )
				{
					_stprintf_s(command, _countof(command), 
						TEXT("Update Rate Set TxSeconds = %I64d, RxSeconds = %I64d ")
						TEXT("Where ProcessUid = %d And Rate = %d;"), 
						it->second.txRate[i], it->second.rxRate[i], puid, i);

					SQLite::Exec(command, true);
				}
			}
		}
	}

	// Flush
	SQLite::Flush();
}

void StatisticsView::InitDatabase()
{
	SQLiteRow protocolRow;
	SQLiteRow packetSizeRow;
	SQLiteRow rateRow;
	TCHAR command[256];

	// Process Protocol
	_stprintf_s(command, _countof(command), TEXT("Select * From Protocol;"));

	protocolRow.InsertType(SQLiteRow::TYPE_INT32); // 0 ProcessUid
	protocolRow.InsertType(SQLiteRow::TYPE_INT32); // 1 Protocol
	protocolRow.InsertType(SQLiteRow::TYPE_INT64); // 2 TxBytes
	protocolRow.InsertType(SQLiteRow::TYPE_INT64); // 3 RxBytes
	protocolRow.InsertType(SQLiteRow::TYPE_INT64); // 4 TxPackets
	protocolRow.InsertType(SQLiteRow::TYPE_INT64); // 5 RxPackets

	SQLite::Select(command, &protocolRow, InitDatabaseProtocolCallback);

	// Process Packet Size
	_stprintf_s(command, _countof(command), TEXT("Select * From PacketSize Where TxPackets > 0 Or RxPackets > 0;"));

	packetSizeRow.InsertType(SQLiteRow::TYPE_INT32); // 0 ProcessUid
	packetSizeRow.InsertType(SQLiteRow::TYPE_INT32); // 1 PacketSize
	packetSizeRow.InsertType(SQLiteRow::TYPE_INT64); // 2 TxBytes
	packetSizeRow.InsertType(SQLiteRow::TYPE_INT64); // 3 RxBytes
	packetSizeRow.InsertType(SQLiteRow::TYPE_INT64); // 4 TxPackets
	packetSizeRow.InsertType(SQLiteRow::TYPE_INT64); // 5 RxPackets

	SQLite::Select(command, &packetSizeRow, InitDatabasePacketSizeCallback);

	// Process Rate
	_stprintf_s(command, _countof(command), TEXT("Select * From Rate Where TxSeconds > 0 Or RxSeconds > 0;"));

	rateRow.InsertType(SQLiteRow::TYPE_INT32); // 0 ProcessUid
	rateRow.InsertType(SQLiteRow::TYPE_INT32); // 1 Rate
	rateRow.InsertType(SQLiteRow::TYPE_INT64); // 2 TxSeconds
	rateRow.InsertType(SQLiteRow::TYPE_INT64); // 3 RxSeconds

	SQLite::Select(command, &rateRow, InitDatabaseRateCallback);
}

void StatisticsView::InitDatabaseProtocolCallback(SQLiteRow *row)
{
	int puid          = row->GetDataInt32(0);
	int protocol      = row->GetDataInt32(1); // Higher 16 bit for exMonth (Jan 1970 = 0), lower 16 bit for mday
	__int64 txBytes   = row->GetDataInt64(2);
	__int64 rxBytes   = row->GetDataInt64(3);
	__int64 txPackets = row->GetDataInt64(4);
	__int64 rxPackets = row->GetDataInt64(5);

	// Insert an StViewItem if PUID not Exist
	if( _items.count(puid) == 0 )
	{
		_items[puid] = StViewItem();
	}

	// Update
	StViewItem &svItem = _items[puid];
	StViewItem &svItemAll = _items[PROCESS_ALL];

	switch ( protocol )
	{
	case TRA_TCP:
		svItem.tx.tcpBytes = txBytes;
		svItem.rx.tcpBytes = rxBytes;
		svItem.tx.tcpPackets = txPackets;
		svItem.rx.tcpPackets = rxPackets;

		svItemAll.tx.tcpBytes += txBytes;
		svItemAll.rx.tcpBytes += rxBytes;
		svItemAll.tx.tcpPackets += txPackets;
		svItemAll.rx.tcpPackets += rxPackets;
		break;

	case TRA_UDP:
		svItem.tx.udpBytes = txBytes;
		svItem.rx.udpBytes = rxBytes;
		svItem.tx.udpPackets = txPackets;
		svItem.rx.udpPackets = rxPackets;

		svItemAll.tx.udpBytes += txBytes;
		svItemAll.rx.udpBytes += rxBytes;
		svItemAll.tx.udpPackets += txPackets;
		svItemAll.rx.udpPackets += rxPackets;
		break;

	case TRA_ICMP:
		svItem.tx.icmpBytes = txBytes;
		svItem.rx.icmpBytes = rxBytes;
		svItem.tx.icmpPackets = txPackets;
		svItem.rx.icmpPackets = rxPackets;

		svItemAll.tx.icmpBytes += txBytes;
		svItemAll.rx.icmpBytes += rxBytes;
		svItemAll.tx.icmpPackets += txPackets;
		svItemAll.rx.icmpPackets += rxPackets;
		break;

	case TRA_IGMP:
		svItem.tx.igmpBytes = txBytes;
		svItem.rx.igmpBytes = rxBytes;
		svItem.tx.igmpPackets = txPackets;
		svItem.rx.igmpPackets = rxPackets;

		svItemAll.tx.igmpBytes += txBytes;
		svItemAll.rx.igmpBytes += rxBytes;
		svItemAll.tx.igmpPackets += txPackets;
		svItemAll.rx.igmpPackets += rxPackets;
		break;

	case TRA_OTHER:
		svItem.tx.otherBytes = txBytes;
		svItem.rx.otherBytes = rxBytes;
		svItem.tx.otherPackets = txPackets;
		svItem.rx.otherPackets = rxPackets;

		svItemAll.tx.otherBytes += txBytes;
		svItemAll.rx.otherBytes += rxBytes;
		svItemAll.tx.otherPackets += txPackets;
		svItemAll.rx.otherPackets += rxPackets;
		break;

	default:
		break;
	}
}

void StatisticsView::InitDatabasePacketSizeCallback(SQLiteRow *row)
{
	int puid          = row->GetDataInt32(0);
	int packetSize    = row->GetDataInt32(1); // [0, 1500]: 1 to 1501+
	__int64 txBytes   = row->GetDataInt64(2);
	__int64 rxBytes   = row->GetDataInt64(3);
	__int64 txPackets = row->GetDataInt64(4);
	__int64 rxPackets = row->GetDataInt64(5);

	// Insert an StViewItem if PUID not Exist
	if( _items.count(puid) == 0 )
	{
		_items[puid] = StViewItem();
	}

	// Update
	StViewItem &svItem = _items[puid];
	StViewItem &svItemAll = _items[PROCESS_ALL];

	svItem.txPacketSize[packetSize] = txPackets;
	svItem.rxPacketSize[packetSize] = rxPackets;
	svItem.txPrevPacketSize[packetSize] = txPackets;
	svItem.rxPrevPacketSize[packetSize] = rxPackets;

	svItemAll.txPacketSize[packetSize] += txPackets;
	svItemAll.rxPacketSize[packetSize] += rxPackets;
	svItemAll.txPrevPacketSize[packetSize] += txPackets;
	svItemAll.rxPrevPacketSize[packetSize] += rxPackets;
}

void StatisticsView::InitDatabaseRateCallback(SQLiteRow *row)
{
	int puid          = row->GetDataInt32(0);
	int rate          = row->GetDataInt32(1); // [0, 1024]: 0 to 1024+
	__int64 txSeconds = row->GetDataInt64(2);
	__int64 rxSeconds = row->GetDataInt64(3);

	// Insert an StViewItem if PUID not Exist
	if( _items.count(puid) == 0 )
	{
		_items[puid] = StViewItem();
	}

	// Update
	StViewItem &svItem = _items[puid];
	StViewItem &svItemAll = _items[PROCESS_ALL];

	svItem.txRate[rate] = txSeconds;
	svItem.rxRate[rate] = rxSeconds;
	svItem.txPrevRate[rate] = txSeconds;
	svItem.rxPrevRate[rate] = rxSeconds;

	svItemAll.txRate[rate] += txSeconds;
	svItemAll.rxRate[rate] += rxSeconds;
	svItemAll.txPrevRate[rate] += txSeconds;
	svItemAll.rxPrevRate[rate] += rxSeconds;
}

void StatisticsView::InsertPacket(PacketInfoEx *pi)
{
	EnterCriticalSection(&_stCS);

	// Insert a StViewItem if PUID not Exist
	if( _items.count(pi->puid) == 0 )
	{
		_items[pi->puid] = StViewItem();
		_items[pi->puid].newItem = true;
		_tcscpy_s(_items[pi->puid].processName, MAX_PATH, pi->name);
	}

	// Create a Reference for Short
	StViewItem &item = _items[pi->puid];
	StViewItem &itemAll = _items[PROCESS_ALL];

	// Update Statistics
	if( pi->dir == DIR_UP )
	{
		switch ( pi->trasportProtocol )
		{
		case TRA_TCP:
			item.tx.tcpBytes += pi->size;
			itemAll.tx.tcpBytes += pi->size;
			break;

		case TRA_UDP:
			item.tx.udpBytes += pi->size;
			itemAll.tx.udpBytes += pi->size;
			break;

		case TRA_ICMP:
			item.tx.icmpBytes += pi->size;
			itemAll.tx.icmpBytes += pi->size;
			break;

		case TRA_IGMP:
			item.tx.igmpBytes += pi->size;
			itemAll.tx.igmpBytes += pi->size;
			break;

		case TRA_OTHER:
			item.tx.otherBytes += pi->size;
			itemAll.tx.otherBytes += pi->size;
			break;

		default:
			break;
		}

		// Set Size Range to [1, 1501]
		int size = pi->size;

		size = ( size <= 1 ) ? 1 : size;
		size = ( size >= 1501 ) ? 1501 : size;

		item.txPacketSize[size - 1] += 1;
		itemAll.txPacketSize[size - 1] += 1;
	}
	else if( pi->dir == DIR_DOWN )
	{
		switch ( pi->trasportProtocol )
		{
		case TRA_TCP:
			item.rx.tcpBytes += pi->size;
			itemAll.rx.tcpBytes += pi->size;
			break;

		case TRA_UDP:
			item.rx.udpBytes += pi->size;
			itemAll.rx.udpBytes += pi->size;
			break;

		case TRA_ICMP:
			item.rx.icmpBytes += pi->size;
			itemAll.rx.icmpBytes += pi->size;
			break;

		case TRA_IGMP:
			item.rx.igmpBytes += pi->size;
			itemAll.rx.igmpBytes += pi->size;
			break;

		case TRA_OTHER:
			item.rx.otherBytes += pi->size;
			itemAll.rx.otherBytes += pi->size;
			break;

		default:
			break;
		}

		// Set Size Range to [1, 1501]
		int size = pi->size;

		size = ( size <= 1 ) ? 1 : size;
		size = ( size >= 1501 ) ? 1501 : size;

		item.rxPacketSize[size - 1] += 1;
		itemAll.rxPacketSize[size - 1] += 1;
	}

	LeaveCriticalSection(&_stCS);
}

void StatisticsView::SetProcessUid(int puid, TCHAR *processName)
{
	EnterCriticalSection(&_stCS);

	_process = puid;

	DrawGraph();

	LeaveCriticalSection(&_stCS);
}

void StatisticsView::TimerProc(HWND hWnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
{
	EnterCriticalSection(&_stCS);

	// Update statistics for rate
	for(std::map<int, StViewItem>::iterator it = _items.begin(); it != _items.end(); ++it)
	{
		int puid = it->first;

		if( puid == PROCESS_ALL )
		{
			continue;
		}

		StViewItem &item = _items[puid];
		StViewItem &itemAll = _items[PROCESS_ALL];
		int txRate;
		int rxRate;

		if( Process::IsProcessActive(puid))
		{
			Process::GetProcessRate(puid, &txRate, &rxRate);

			txRate /= 1024; // Unit KB/s
			rxRate /= 1024; 

			txRate = txRate > 1024 ? 1024 : txRate; // Limit 1024 KB/s
			rxRate = rxRate > 1024 ? 1024 : rxRate;

			item.txRate[txRate] += 1;
			item.rxRate[rxRate] += 1;

			itemAll.txRate[txRate] += 1;
			itemAll.rxRate[rxRate] += 1;
		}
	}

	// Start Painting
	DrawGraph();

	LeaveCriticalSection(&_stCS);
}

void StatisticsView::DrawGraph()
{
	//  -------------------    -------------------    -------------------
	// Protocol            |  PacketSize          |  Rate                |
	// |                   |  |                   |  |                   |
	// |                   |  |                   |  |                   |
	// |                   |  |                   |  |                   |
	// |                   |  |                   |  |                   |
	// |                   |  |                   |  |                   |
	// |                   |  |                   |  |                   |
	// |                   |  |                   |  |                   |
	//  -------------------    -------------------    -------------------
	// Rectangle for protocol box
	int ptl_x      = 12;
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

	StViewItem &item = _items[_process];

	// Clear Background
	Rectangle(_hdcBuf, -1, -1, _width + 1, _height + 1);

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
/*
	GwHistogram txPszHistogram(_hdcBuf, pszct_x, pszct_y, pszct_width, pszct_height / 2, item.txPacketSize, 1501, pszScales, 4, "Tx", RGB(0xDF, 0x00, 0x24));
	GwHistogram rxPszHistogram(_hdcBuf, pszct_x, pszct_y + pszct_height / 2, pszct_width, pszct_height / 2, item.rxPacketSize, 1501, pszScales, 4, "Rx", RGB(0x31, 0x77, 0xC1));
*/
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
/*
	GwHistogram txRteHistogram(_hdcBuf, 
		rtect_x, rtect_y, rtect_width, rtect_height / 2, 
		item.txRate, 1025, rteScales, 3, "Tx", RGB(0xDF, 0x00, 0x24));
	GwHistogram rxRteHistogram(_hdcBuf, 
		rtect_x, rtect_y + rtect_height / 2, rtect_width, rtect_height / 2, 
		item.rxRate, 1025, rteScales, 3, "Rx", RGB(0x31, 0x77, 0xC1));
*/
	GwLogHistogram txRteHistogram(_hdcBuf, 
		rtect_x, rtect_y, rtect_width, rtect_height / 2, 
		item.txRate, 1025, rteScales, 3, Language::GetString(IDS_STVIEW_TX), RGB(0xDF, 0x00, 0x24), 4, 4);
	GwLogHistogram rxRteHistogram(_hdcBuf, 
		rtect_x, rtect_y + rtect_height / 2, rtect_width, rtect_height / 2, 
		item.rxRate, 1025, rteScales, 3, Language::GetString(IDS_STVIEW_RX), RGB(0x31, 0x77, 0xC1), 4, 4);

	txRteHistogram.Paint();
	rxRteHistogram.Paint();

	// Wriet to Screen
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
