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
#include "PortCache.h"

PortCache::PortCache()
{
    RtlZeroMemory(_tcpPortTable, sizeof(_tcpPortTable));
    RtlZeroMemory(_udpPortTable, sizeof(_udpPortTable));
}

int PortCache::GetTcpPortPid(int port)
{
    if( _tcpPortTable[port] != 0 )
    {
        return _tcpPortTable[port];
    }
    else
    {
        // Rebuild Cache
        RebuildTcpTable();
        
        // Return
        return _tcpPortTable[port];
    }
}

int PortCache::GetUdpPortPid(int port)
{
    if( _udpPortTable[port] != 0 )
    {
        return _udpPortTable[port];
    }
    else
    {
        // Rebuild Cache
        RebuildUdpTable();

        // Return
        return _udpPortTable[port];
    }
}

//原本的，由于ANY_SIZE在当前项目下默认为1，所以假设注释掉了
//#define  ANY_SIZE 1024，那么此程序将无法正常获取活动过程
//中的网络进程信息，参照MSDN里GetTcpTable的用法改写了此函数，
//如下所示：
void PortCache::RebuildTcpTable()
{
	// Clear the table
	RtlZeroMemory(_tcpPortTable, sizeof(_tcpPortTable));

	DWORD dwRetValue = NO_ERROR;
	DWORD dwBufferSize = 0;

	PMIB_TCPTABLE_OWNER_PID pTable = NULL;
	//第一次获取，尝试获得pTable所需的内存大小，用dwBufferSize存放起来
	dwRetValue = GetExtendedTcpTable(NULL, &dwBufferSize,
		FALSE, AF_INET, TCP_TABLE_OWNER_PID_ALL, 0);
	if (dwRetValue != ERROR_INSUFFICIENT_BUFFER)
	{
		printf("Failed to snapshot TCP endpoints.\n");
		return;
	}

	//使用HeapAlloc在堆上分配一个dwBufferSize大小的内存空间给pTable变量
	pTable = (PMIB_TCPTABLE_OWNER_PID)HeapAlloc(GetProcessHeap(), 0, dwBufferSize);
	//第二次获取，将TCP相关的网络进程快照保存到pTable变量中
	dwRetValue = GetExtendedTcpTable(pTable, &dwBufferSize,
		FALSE, AF_INET, TCP_TABLE_OWNER_PID_ALL, 0);

	if (dwRetValue != NO_ERROR)
	{
		printf("Failed to snapshot TCP endpoints.\n");
		HeapFree(GetProcessHeap(), 0, pTable);
		return;
	}

	// Rebuild the table
	for (unsigned int i = 0; i < pTable->dwNumEntries; i++)
	{
		_tcpPortTable[ntohs((unsigned short)pTable->table[i].dwLocalPort)] =
			pTable->table[i].dwOwningPid;
	}
	HeapFree(GetProcessHeap(), 0, pTable);
}

void PortCache::RebuildUdpTable()
{
	// Clear the table
	RtlZeroMemory(_udpPortTable, sizeof(_udpPortTable));

	DWORD dwRetValue = NO_ERROR;
	DWORD dwBufferSize = 0;

	//解释如同RebuildTcpTable
	PMIB_UDPTABLE_OWNER_PID pTable = NULL;
	dwRetValue = GetExtendedUdpTable(NULL, &dwBufferSize,
		FALSE, AF_INET, UDP_TABLE_OWNER_PID, 0);
	if (dwRetValue != ERROR_INSUFFICIENT_BUFFER)
	{
		printf("Failed to snapshot UDP endpoints.\n");
		return;
	}

	//解释如同RebuildTcpTable
	pTable = (PMIB_UDPTABLE_OWNER_PID)HeapAlloc(GetProcessHeap(), 0, dwBufferSize);
	dwRetValue = GetExtendedUdpTable(pTable, &dwBufferSize,
		FALSE, AF_INET, UDP_TABLE_OWNER_PID, 0);

	if (dwRetValue != NO_ERROR)
	{
		printf("Failed to snapshot UDP endpoints.\n");
		HeapFree(GetProcessHeap(), 0, pTable);
		return;
	}

	// Rebuild the table
	for (unsigned int i = 0; i < pTable->dwNumEntries; i++)
	{
		_udpPortTable[ntohs((unsigned short)pTable->table[i].dwLocalPort)] =
			pTable->table[i].dwOwningPid;
	}
	HeapFree(GetProcessHeap(), 0, pTable);
}