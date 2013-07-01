#include <windows.h>
#include <tchar.h>

// RegUpdater
// ------------------
// Usage: 
//    RegUpdater [NetmonFullPath | -c]
//    If Netmon's full path is not provided, the autorun registry item will be removed
//
// Example
//    Enable AutoRun:  RegUpdater "D:\Netmon.exe"
//    Disable AutoRun: RegUpdater -c
int WINAPI _tWinMain(HINSTANCE hInstance, HINSTANCE, LPTSTR lpCmdLine, int nCmdShow)
{
	HKEY hRunKey;
	BOOL bAutoRunExist = FALSE;

	// Open registry key
	if( RegOpenKeyEx(HKEY_CURRENT_USER, TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Run"), 
		0, KEY_QUERY_VALUE | KEY_SET_VALUE, &hRunKey) != 0 )
	{
		return 1;
	}

	// See if an autorun item exists
	if( RegGetValue(hRunKey, 0, TEXT("Netmon"), RRF_RT_REG_SZ, 0, 0, 0) == 0 )
	{
		bAutoRunExist = TRUE;
	}

	// Update key value
	if( lpCmdLine[0] == TEXT('-') && 
		lpCmdLine[1] == TEXT('c') && 
		lpCmdLine[2] == 0 ) // Disable AutoRun
	{
		if( bAutoRunExist )
		{
			RegDeleteValue(hRunKey, TEXT("Netmon"));
		}
		else // No need to delete value
		{
		}
	}
	else if( lpCmdLine[0] != 0 )// Enable AutoRun
	{
		TCHAR cmd[MAX_PATH];
		_stprintf_s(cmd, MAX_PATH, TEXT("\"%s\" -h"), lpCmdLine);
		RegSetValueEx(hRunKey, TEXT("Netmon"), 0, REG_SZ, (BYTE *)cmd, _tcslen(cmd) * sizeof(TCHAR) + 1);
	}
	else // No argument, do nothing
	{
	}

	//  Close registry key
	RegCloseKey(hRunKey);

	// Exit
	return 0;
}