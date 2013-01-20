#include "stdafx.h"
#include "ProcessCache.h"

ProcessCache *ProcessCache::_instance = NULL;

ProcessCache::ProcessCache()
{
	InitializeCriticalSection(&_cs);
	RtlZeroMemory(_nameTable, sizeof(_nameTable));
	RtlZeroMemory(_pathTable, sizeof(_pathTable));
}

ProcessCache *ProcessCache::instance()
{
	if (_instance == NULL)
		_instance = new ProcessCache();
	return _instance;
}

TCHAR *ProcessCache::GetName(int pid)
{
	TCHAR *result;
	EnterCriticalSection(&_cs);

	if (_nameTable[pid][0] == TEXT('\0'))
	{
		rebuildTable();
	}
	result = _nameTable[pid][0] == TEXT('\0') ? TEXT("Unknown") : _nameTable[pid];

	LeaveCriticalSection(&_cs);
	return result;
}

TCHAR *ProcessCache::GetFullPath(int pid)
{
	TCHAR *result;
	EnterCriticalSection(&_cs);

	if (_pathTable[pid][0] == TEXT('\0'))
	{
		rebuildTable();
	}
	result = _pathTable[pid][0] == TEXT('\0') ? TEXT("-") : _pathTable[pid];

	LeaveCriticalSection(&_cs);
	return result;
}

BOOL ProcessCache::IsProcessAlive(int pid, const TCHAR *name)
{
	BOOL result;
	EnterCriticalSection(&_cs);

	rebuildTable();
	result =  (_tcscmp(_nameTable[pid], name) == 0) ? TRUE : FALSE;

	LeaveCriticalSection(&_cs);
	return result;
}

void ProcessCache::rebuildTable()
{
	EnterCriticalSection(&_cs);

	// Clear Tables
	RtlZeroMemory(_nameTable, sizeof(_nameTable));
	RtlZeroMemory(_pathTable, sizeof(_pathTable));

	// Take a snapshot
	HANDLE hSnapShot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	PROCESSENTRY32 p;
	p.dwSize = sizeof(PROCESSENTRY32);

	// Traverse Process List
	for(BOOL ret = Process32First(hSnapShot, &p); ret != 0; ret = Process32Next(hSnapShot, &p))
	{
		// Get pid and file name
		int pid = p.th32ProcessID;
		TCHAR *processName = p.szExeFile;

		// Get full path (if possible)
		HANDLE hProcess = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION | PROCESS_VM_READ, FALSE, pid);
		if (hProcess == 0)
		{
			_tcscpy_s(_nameTable[pid], MAX_PATH, processName);
			_tcscpy_s(_pathTable[pid], MAX_PATH, TEXT("-"));
		}
		else
		{
			TCHAR fullPath[260];
			GetModuleFileNameEx(hProcess, 0, fullPath, 260);
			_tcscpy_s(_nameTable[pid], MAX_PATH, processName);
			_tcscpy_s(_pathTable[pid], MAX_PATH, fullPath);
		}
		CloseHandle(hProcess);
	}
	CloseHandle(hSnapShot);

	LeaveCriticalSection(&_cs);
}
