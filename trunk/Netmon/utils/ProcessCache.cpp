#include "stdafx.h"
#include "Utils.h"
#include "ProcessCache.h"

ProcessCache *ProcessCache::_instance = NULL;

ProcessCache::ProcessCache()
{
	InitializeCriticalSection(&_cs);

	for (int i = 0; i < 65536 / 4; i++)
	{
		_nameTable[i] = new TCHAR[MAX_PATH];
		_pathTable[i] = new TCHAR[MAX_PATH];
		RtlZeroMemory(_nameTable[i], MAX_PATH * sizeof(TCHAR));
		RtlZeroMemory(_pathTable[i], MAX_PATH * sizeof(TCHAR));
	}
}

ProcessCache::~ProcessCache()
{
	DeleteCriticalSection(&_cs);
}

ProcessCache *ProcessCache::instance()
{
	if (_instance == NULL)
		_instance = new ProcessCache();
	return _instance;
}

void ProcessCache::GetName(int pid, TCHAR *buf, int cchLen)
{
	assert(pid >= 0 && pid < 65536);

	EnterCriticalSection(&_cs);
	
	if (_tcslen(_nameTable[pid / 4]) == 0)
	{
		rebuildTable();
	}
	_tcscpy_s(buf, cchLen, _nameTable[pid / 4]);

	LeaveCriticalSection(&_cs);
}

void ProcessCache::GetFullPath(int pid, TCHAR *buf, int cchLen)
{
	assert(pid >= 0 && pid < 65536);

	EnterCriticalSection(&_cs);

	if (_tcslen(_pathTable[pid / 4]) == 0)
	{
		rebuildTable();
	}
	_tcscpy_s(buf, cchLen, _pathTable[pid / 4]);

	LeaveCriticalSection(&_cs);
}

BOOL ProcessCache::IsProcessAlive(int pid, const TCHAR *name, bool rebuild)
{
	assert(pid >= 0 && pid < 65536);

	BOOL result;
	EnterCriticalSection(&_cs);

	if (rebuild)
	{
		rebuildTable(false);
	}
	result =  (_tcscmp(_nameTable[pid / 4], name) == 0) ? TRUE : FALSE;

	LeaveCriticalSection(&_cs);
	return result;
}

void ProcessCache::rebuildTable(bool dump)
{
	// Clear Tables
	for (int i = 0; i < 65536 / 4; i++)
	{
		RtlZeroMemory(_nameTable[i], MAX_PATH * 2);
		RtlZeroMemory(_pathTable[i], MAX_PATH * 2);
	}

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

		assert(pid >= 0 && pid < 65536);

		// Get full path (if possible)
		HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid);
		if (hProcess == 0)
		{
			_tcscpy_s(_nameTable[pid / 4], MAX_PATH, processName);
			_tcscpy_s(_pathTable[pid / 4], MAX_PATH, TEXT("-"));
			if (dump)
			{
				Utils::DbgPrint(TEXT("   PID = %d, Name = \"%s\", FullPath = \"%s\"\n"), pid, processName, TEXT("-"));
			}
		}
		else
		{
			TCHAR fullPath[MAX_PATH];
			_tcscpy_s(_nameTable[pid / 4], MAX_PATH, processName);
			if (GetModuleFileNameEx(hProcess, 0, fullPath, MAX_PATH) > 0) // Success
			{
				_tcscpy_s(_pathTable[pid / 4], MAX_PATH, fullPath);
			}
			else
			{
				_tcscpy_s(_pathTable[pid / 4], MAX_PATH, TEXT("-"));
			}
		}
		CloseHandle(hProcess);
	}
	CloseHandle(hSnapShot);
}
