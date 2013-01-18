#ifndef DETAIL_VIEW_H
#define DETAIL_VIEW_H

#include "NetView.h"

class DetailView : public NetView
{
protected:
	typedef struct tagDtViewItem
	{
		__int64 curPackets;
		__int64 prevPackets;

		TCHAR processName[MAX_PATH];

		struct tagDtViewItem()
		{
			curPackets = 0;
			prevPackets = 0;
			RtlZeroMemory(processName, sizeof(processName));
		}
	} DtViewItem;

	static std::map<int, DtViewItem> _items;
	static HWND _hWnd;
	static HWND _hList;
	static int _iLanguageId;
	static __int64 _curPage;
	static WNDPROC _lpOldProcEdit;
	static CRITICAL_SECTION _stCS;

protected:
	static LRESULT CALLBACK MyProcEdit(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	static void WINAPI TimerProc(HWND hWnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime);
	static void InitDatabase();
	static void SaveDatabase();

	static void UpdateSize(HWND hWnd);
	static void UpdateContent(bool rebuildList);
	static void UpdateContentCallback(SQLiteRow *row);

	static void ListViewInsert(int uid, int puid, int dir, int protocol, int size, __int64 time, int port);

	static void OnPageUp();
	static void OnPageDown();
	static void OnGoto();

	static void SwitchLanguage(HWND hWnd);

public:
	virtual void Init();
	virtual void End();
	virtual void InsertPacket(PacketInfoEx *pi);
	virtual void SetProcessUid(int puid, TCHAR *processName);

	virtual LRESULT DlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	__int64 GetPacketCount();
	void OnAllPacketsDeleted();
};

#endif
