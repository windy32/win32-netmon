#ifndef PROCESS_H
#define PROCESS_H

#include "SQLite.h"
#include "Packet.h"

class Process
{
protected:
	// The Process class maintains a list of process items
	typedef struct tagProcessItem
	{
		// Static Parameters
		bool active;

		int  puid;
		TCHAR name[MAX_PATH];
		TCHAR fullPath[MAX_PATH];

		// Valid Only When Active
		HANDLE handle;

		bool dirty;

		int txRate;
		int rxRate;
		int prevTxRate;
		int prevRxRate;

		int pauid;
		int startTime;
		int endTime;
	} ProcessItem;

	static std::vector<ProcessItem> _processes;

	// It's associated with a ListView control
	static HWND _hList;

protected:
	static void InitCallback(SQLiteRow *row);

	static void ListViewInsert(int index, HWND hList);
	static void ListViewUpdate(int index);

public:
	static void Init(HWND hList);
	static void OnPacket(PacketInfoEx *pi);
	static void OnTimer();
	static void OnExit();

public:
	static int  GetProcessUid(int index);
	static int  GetProcessCount();
	static int  GetProcessUid(const TCHAR *name);
	static bool GetProcessName(int puid, TCHAR *buf, int len);
	static int  GetProcessIndex(int puid);
	static bool GetProcessRate(int puid, int *txRate, int *rxRate);
	static bool IsProcessActive(int puid);
};

#endif