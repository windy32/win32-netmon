// Copyright (C) 2012-2014 F32 (feng32tc@gmail.com)
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License version 3 as
// published by the Free Software Foundation;
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA 

#include "stdafx.h"
#include "MainWnd.h"

// Language
#include "../Lang/resource.h"
#include "utils/Language.h"
int            g_nLanguage;
int            g_iCurLanguage;
HINSTANCE g_hInstance;

// Options -h
bool    g_bHideWindow = false;
///----------------------------------------------------------------------------------------------//
///                                    WinMain Entry                                             //
///----------------------------------------------------------------------------------------------//
int __stdcall WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR lpCmdLine, int nCmdShow)
{
	ILog4zManager::getRef().setLoggerPath(LOG4Z_MAIN_LOGGER_ID, "NetMonLog");
	ILog4zManager::getRef().start();
	ILog4zManager::getRef().setLoggerLevel(LOG4Z_MAIN_LOGGER_ID, LOG_LEVEL_TRACE);

	LOGT(_T("开始运行主程序"));

    // Single Instance (Create a Named-Pipe)
	LOGT(_T("创建匿名通道，为了防止程序重复执行"));
    HANDLE hPipe = CreateNamedPipe(_T("\\\\.\\pipe\\netmon"), 
        PIPE_ACCESS_DUPLEX | FILE_FLAG_FIRST_PIPE_INSTANCE, 0, 4, 1024, 1024, 1000, NULL);
    if( hPipe == INVALID_HANDLE_VALUE )
    {
        MessageBox(0, _T("Netmon is still running.\nOnly one instance is allowed for Netmon!"), 
            _T("Error"), MB_OK | MB_ICONWARNING);
        return 1;
    }

    // Load languages
	LOGT(_T("加载语言模块"));
    g_nLanguage = Language::Load();
    if( g_nLanguage == 0 )
    {
        MessageBox(0, _T("Failed to load languages."), _T("Error"), MB_OK | MB_ICONWARNING);
        CloseHandle(hPipe);
        return 1;
    }
    else // Select English as default if available
    {
		TCHAR szEnglishName[64] = { 0 };
        TCHAR szNativeName[64] = { 0 };
        for (int i = 0; i < g_nLanguage; i++)
        {
            Language::GetName(i, szEnglishName, 64, szNativeName, 64);
            if (_tcscmp(szEnglishName, _T("English")) == 0)
            {
                g_iCurLanguage = i;
                break;
            }
        }
        Language::Select(g_iCurLanguage);
    }

    // Option "-h"
    if (strcmp(lpCmdLine, "-h") == 0)
    {
        g_bHideWindow = true;
    }

	g_hInstance = hInstance;
	MainWindow MainWnd(hInstance);
	MainWnd.DisplayWindow();
	LRESULT lReslut = MainWnd.LoopMessage();

	// Close the Named-Pipe
	LOGT(_T("关闭匿名通道"));
	CloseHandle(hPipe);

	LOGT(_T("退出主程序"));
	ILog4zManager::getRef().stop();
    // Exit
    return lReslut;
}