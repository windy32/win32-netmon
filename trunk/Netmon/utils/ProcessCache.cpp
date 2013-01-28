#include "stdafx.h"
#include "Utils.h"
#include "ProcessCache.h"

ProcessCache *ProcessCache::_instance = NULL;

ProcessCache::ProcessCache()
{
	InitializeCriticalSection(&_cs);
	RtlZeroMemory(_nameTable, sizeof(_nameTable));
	RtlZeroMemory(_pathTable, sizeof(_pathTable));
	_csCounter = 0; // For debugging
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

TCHAR *ProcessCache::GetName(int pid)
{
	TCHAR *result;
	EnterCriticalSection(&_cs);
	//Utils::DbgPrint(TEXT("ProcessCache::GetName %d"), ++_csCounter); // For debugging
	
	if (_nameTable[pid / 4][0] == TEXT('\0'))
	{
		Utils::DbgPrint(TEXT("Rebuild Table...")); // For debugging
		rebuildTable();
		result = _nameTable[pid / 4];
		if (result[0] == TEXT('\0'))
		{
			Utils::DbgPrint(TEXT("PID %d Not Found"), pid);
		}
		else
		{
			Utils::DbgPrint(TEXT("PID %d : \"%s\""), pid, _nameTable[pid / 4]);
		}
	}
	result = _nameTable[pid / 4][0] == TEXT('\0') ? TEXT("Unknown") : _nameTable[pid / 4];

	LeaveCriticalSection(&_cs);
	return result;
}

TCHAR *ProcessCache::GetFullPath(int pid)
{
	TCHAR *result;
	EnterCriticalSection(&_cs);
	//Utils::DbgPrint(TEXT("ProcessCache::GetFullPath %d"), ++_csCounter); // For debugging

	if (_pathTable[pid / 4][0] == TEXT('\0'))
	{
		Utils::DbgPrint(TEXT("Rebuild Table...")); // For debugging
		rebuildTable();
		result = _pathTable[pid / 4];
		if (result[0] == TEXT('\0'))
		{
			Utils::DbgPrint(TEXT("PID %d Not Found"), pid);
		}
		else
		{
			Utils::DbgPrint(TEXT("PID %d : \"%s\""), pid, _pathTable[pid / 4]);
		}
	}
	result = _pathTable[pid / 4][0] == TEXT('\0') ? TEXT("-") : _pathTable[pid / 4];

	LeaveCriticalSection(&_cs);
	return result;
}

BOOL ProcessCache::IsProcessAlive(int pid, const TCHAR *name)
{
	BOOL result;
	EnterCriticalSection(&_cs);
	//Utils::DbgPrint(TEXT("ProcessCache::IsProcessAlive %d"), ++_csCounter); // For debugging

	rebuildTable();
	result =  (_tcscmp(_nameTable[pid / 4], name) == 0) ? TRUE : FALSE;

	if (result == TRUE) // For debugging
	{
		Utils::DbgPrint(TEXT("PID %d, \"%s\" is alive"), pid, name);
	}
	else
	{
		Utils::DbgPrint(TEXT("PID %d, \"%s\" is dead, new name is %s"), pid, name, _nameTable[pid / 4]);
	}

	LeaveCriticalSection(&_cs);
	return result;
}

void ProcessCache::rebuildTable()
{
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
		HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid);
		if (hProcess == 0)
		{
			_tcscpy_s(_nameTable[pid / 4], MAX_PATH, processName);
			_tcscpy_s(_pathTable[pid / 4], MAX_PATH, TEXT("-"));
			Utils::DbgPrint(TEXT("   PID = %d, Name = \"%s\", FullPath = \"%s\""), pid, processName, TEXT("-"));
		}
		else
		{
			TCHAR fullPath[260];
			_tcscpy_s(_nameTable[pid / 4], MAX_PATH, processName);
			if (GetModuleFileNameEx(hProcess, 0, fullPath, 260) > 0) // Success
			{
				_tcscpy_s(_pathTable[pid / 4], MAX_PATH, fullPath);
			}
			else
			{
				_tcscpy_s(_pathTable[pid / 4], MAX_PATH, TEXT("-"));
			}
			Utils::DbgPrint(TEXT("   PID = %d, Name = \"%s\", FullPath = \"%s\""), pid, processName, fullPath);
		}
		CloseHandle(hProcess);
	}
	CloseHandle(hSnapShot);
}
