#ifndef STATISTICS_VIEW_H
#define STATISTICS_VIEW_H

#include "NetView.h"

class StatisticsView : public NetView
{
protected:
	// The Item of a Process ------------------------------
	typedef struct tagStViewItem
	{
		struct tagStCountItem
		{
			__int64 tcpPackets;
			__int64 udpPackets;
			__int64 icmpPackets;
			__int64 igmpPackets;
			__int64 otherPackets;

			__int64 tcpBytes;
			__int64 udpBytes;
			__int64 icmpBytes;
			__int64 igmpBytes;
			__int64 otherBytes;
		} rx, tx;

		__int64 rxPacketSize[1501]; // 1 to 1501+ Bytes
		__int64 txPacketSize[1501];

		__int64 rxPrevPacketSize[1501];
		__int64 txPrevPacketSize[1501];

		__int64 rxRate[1025]; // 0 to 1024+ KB/s
		__int64 txRate[1025];

		__int64 rxPrevRate[1025];
		__int64 txPrevRate[1025];

		bool newItem; // false when the StViewItem is created in InitDatabase

		TCHAR processName[MAX_PATH];

		struct tagStViewItem()
		{
			RtlZeroMemory(processName, sizeof(processName));

			RtlZeroMemory(&rx, sizeof(rx));
			RtlZeroMemory(&tx, sizeof(tx));

			RtlZeroMemory(rxPacketSize, sizeof(rxPacketSize));
			RtlZeroMemory(txPacketSize, sizeof(txPacketSize));

			RtlZeroMemory(rxPrevPacketSize, sizeof(rxPrevPacketSize));
			RtlZeroMemory(txPrevPacketSize, sizeof(txPrevPacketSize));

			RtlZeroMemory(rxRate, sizeof(rxRate));
			RtlZeroMemory(txRate, sizeof(txRate));

			RtlZeroMemory(rxPrevRate, sizeof(rxPrevRate));
			RtlZeroMemory(txPrevRate, sizeof(txPrevRate));

			newItem = false;
		}
	} StViewItem;

	// Items for All Processes ----------------------------
	static std::map<int, StViewItem> _items;

	static HDC     _hdcTarget;
	static HDC     _hdcBuf;
	static HBITMAP _hbmpBuf;

	static StatisticsView * _this;
	static CRITICAL_SECTION _stCS;

protected:
	static void DrawGraph();
	static void WINAPI TimerProc(HWND hWnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime);

	static void SaveDatabase();
	static void InitDatabase();
	static void InitDatabaseProtocolCallback(SQLiteRow *row);
	static void InitDatabasePacketSizeCallback(SQLiteRow *row);
	static void InitDatabaseRateCallback(SQLiteRow *row);

public:
	virtual void Init();
	virtual void End();
	virtual void InsertPacket(PacketInfoEx *pi);
	virtual void SetProcessUid(int puid, TCHAR *processName);

	virtual LRESULT DlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
};

#endif
