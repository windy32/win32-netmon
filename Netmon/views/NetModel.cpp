#include "stdafx.h"
#include "NetModel.h"

const int NetModel::PROCESS_ALL = -1;

NetModel::NetModel()
{
	InitializeCriticalSection(&_cs);
}

NetModel::~NetModel()
{
	DeleteCriticalSection(&_cs);
}

void NetModel::Lock()
{
	EnterCriticalSection(&_cs);
}

void NetModel::Unlock()
{
	LeaveCriticalSection(&_cs);
}
