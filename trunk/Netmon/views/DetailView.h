#ifndef DETAIL_VIEW_H
#define DETAIL_VIEW_H

#include "NetView.h"
#include "DetailModel.h"

class DetailView : public NetView
{
protected:
	// UI Elements & States
	static HWND _hWnd;
	static HWND _hList;
	static int _iLanguageId;
	static __int64 _curPage;
	static WNDPROC _lpOldProcEdit;

	// Model Object
	static DetailModel *_model;

protected:
	static LRESULT CALLBACK MyProcEdit(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	static void WINAPI TimerProc(HWND hWnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime);

	static void UpdateSize(HWND hWnd);
	static void UpdateContent(bool rebuildList);
	static void UpdateContentCallback(SQLiteRow *row);

	static void ListViewInsert(int uid, int puid, int dir, int protocol, int size, __int64 time, int port);

	static void OnPageUp();
	static void OnPageDown();
	static void OnGoto();

	static void SwitchLanguage(HWND hWnd);

public:
	virtual void Init(DetailModel *model);
	virtual void End();
	virtual void SetProcessUid(int puid);

	virtual LRESULT DlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	__int64 GetPacketCount();
	void OnAllPacketsDeleted();
};

#endif
