#ifndef NET_VIEW_H
#define NET_VIEW_H

#include "../utils/Packet.h"
#include "../utils/SQLite.h"
#include "../utils/Language.h"

// Base class of all concrete views
class NetView
{
protected:
	static int     _process;

	static int     _width;
	static int     _height;

	static const int PROCESS_ALL;

public:
	virtual void SetProcessUid(int puid) = 0;
	virtual LRESULT DlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) = 0;
};

#endif
