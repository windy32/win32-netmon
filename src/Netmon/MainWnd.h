#ifndef __MAINWND_H
#define __MAINWND_H
#include <windows.h>

class MainWindow {
private:
	HINSTANCE m_hInstance;
	HWND m_hWnd;

public:
	MainWindow(HINSTANCE hInst)
	{
		m_hInstance = hInst;
		m_hWnd = NULL;
	}

	~MainWindow()
	{

	}

protected:
	static INT_PTR CALLBACK ProcDlgMainEx(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

public:
	void DisplayWindow();
	LRESULT LoopMessage();
};


#endif


