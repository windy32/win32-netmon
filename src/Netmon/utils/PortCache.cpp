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
#pragma warning(disable:4996)

PortCache::PortCache()
{
    //RtlZeroMemory(_tcpPortTable, sizeof(_tcpPortTable));
    //RtlZeroMemory(_udpPortTable, sizeof(_udpPortTable));
}
//
//int PortCache::GetTcpPortPid(int port)
//{
//    if( _tcpPortTable[port] != 0 )
//    {
//        return _tcpPortTable[port];
//    }
//    else
//    {
//        // Rebuild Cache
//        RebuildTcpTable();
//        
//        // Return
//        return _tcpPortTable[port];
//    }
//}
//
//int PortCache::GetUdpPortPid(int port)
//{
//    if( _udpPortTable[port] != 0 )
//    {
//        return _udpPortTable[port];
//    }
//    else
//    {
//        // Rebuild Cache
//        RebuildUdpTable();
//
//        // Return
//        return _udpPortTable[port];
//    }
//}

MIB_TCPROW_OWNER_PID PortCache::GetTcpPortPidEx(int port)
{
	if (_mapTcpPortTableEx.find(port) != _mapTcpPortTableEx.end())
	{
		return _mapTcpPortTableEx[port];
	}
	else {
		// Rebuild Cache
		RebuildTcpTable();

		// Return
		return _mapTcpPortTableEx[port];
	}
}

MIB_UDPROW_OWNER_PID PortCache::GetUdpPortPidEx(int port)
{
	if (_mapUdpPortTableEx.find(port) != _mapUdpPortTableEx.end())
	{
		return _mapUdpPortTableEx[port];
	}
	else {
		// Rebuild Cache
		RebuildUdpTable();

		// Return
		return _mapUdpPortTableEx[port];
	}
}

void PortCache::GetPortStateText(DWORD dwPortState, int nLanguageID, TCHAR szPortState[])
{
	switch (dwPortState)
	{
	case MIB_TCP_STATE_CLOSED:
		_tcscpy(szPortState, nLanguageID == 0 ? _T("�Ѿ��ر�") : _T("CLOSED")); break;
	case MIB_TCP_STATE_LISTEN:
		_tcscpy(szPortState, nLanguageID == 0 ? _T("���ڼ���") : _T("LISTEN")); break;
	case MIB_TCP_STATE_SYN_SENT:
		_tcscpy(szPortState, nLanguageID == 0 ? _T("��������") : _T("SYN_SENT")); break;
	case MIB_TCP_STATE_SYN_RCVD:
		_tcscpy(szPortState, nLanguageID == 0 ? _T("��������") : _T("SYN_RCVD")); break;
	case MIB_TCP_STATE_ESTAB:
		_tcscpy(szPortState, nLanguageID == 0 ? _T("�Ѿ�����") : _T("ESTABLISHED")); break;
	case MIB_TCP_STATE_FIN_WAIT1:
		_tcscpy(szPortState, nLanguageID == 0 ? _T("�ȴ� 1") : _T("FIN_WAIT1")); break;
	case MIB_TCP_STATE_FIN_WAIT2:
		_tcscpy(szPortState, nLanguageID == 0 ? _T("�ȴ� 2") : _T("FIN_WAIT2")); break;
	case MIB_TCP_STATE_CLOSE_WAIT:
		_tcscpy(szPortState, nLanguageID == 0 ? _T("�ȴ��ر�") : _T("CLOSE_WAIT")); break;
	case MIB_TCP_STATE_CLOSING:
		_tcscpy(szPortState, nLanguageID == 0 ? _T("���ڹر�") : _T("CLOSING")); break;
	case MIB_TCP_STATE_LAST_ACK:
		_tcscpy(szPortState, nLanguageID == 0 ? _T("�ȴ��ж�") : _T("LAST_ACK")); break;
	case MIB_TCP_STATE_TIME_WAIT:
		_tcscpy(szPortState, nLanguageID == 0 ? _T("�ȴ���ʱ") : _T("TIME_WAIT")); break;
	case MIB_TCP_STATE_DELETE_TCB:
		_tcscpy(szPortState, nLanguageID == 0 ? _T("ɾ��TCB") : _T("DELETE")); break;
	default:
		break;
	}
}

//ԭ���ģ�����ANY_SIZE�ڵ�ǰ��Ŀ��Ĭ��Ϊ1�����Լ���ע�͵���
//#define  ANY_SIZE 1024����ô�˳����޷�������ȡ�����
//�е����������Ϣ������MSDN��GetTcpTable���÷���д�˴˺�����
//������ʾ��
void PortCache::RebuildTcpTable()
{
	// Clear the table
	//RtlZeroMemory(_tcpPortTable, sizeof(_tcpPortTable));
	_mapTcpPortTableEx.clear();

	DWORD dwRetValue = NO_ERROR;
	DWORD dwBufferSize = 0;

	PMIB_TCPTABLE_OWNER_PID pTable = NULL;
	//��һ�λ�ȡ�����Ի��pTable������ڴ��С����dwBufferSize�������
	dwRetValue = GetExtendedTcpTable(NULL, &dwBufferSize,
		FALSE, AF_INET, TCP_TABLE_OWNER_PID_ALL, 0);
	if (dwRetValue != ERROR_INSUFFICIENT_BUFFER)
	{
		printf("Failed to snapshot TCP endpoints.\n");
		return;
	}

	//ʹ��HeapAlloc�ڶ��Ϸ���һ��dwBufferSize��С���ڴ�ռ��pTable����
	pTable = (PMIB_TCPTABLE_OWNER_PID)HeapAlloc(GetProcessHeap(), 0, dwBufferSize);
	//�ڶ��λ�ȡ����TCP��ص�������̿��ձ��浽pTable������
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
		int nTemp = ntohs((unsigned short)pTable->table[i].dwLocalPort);
		//_tcpPortTable[nTemp] =
		//	pTable->table[i].dwOwningPid;
		_mapTcpPortTableEx[nTemp] = pTable->table[i];
	}
	HeapFree(GetProcessHeap(), 0, pTable);
}

void PortCache::RebuildUdpTable()
{
	// Clear the table
	//RtlZeroMemory(_udpPortTable, sizeof(_udpPortTable));
	_mapUdpPortTableEx.clear();

	DWORD dwRetValue = NO_ERROR;
	DWORD dwBufferSize = 0;

	//������ͬRebuildTcpTable
	PMIB_UDPTABLE_OWNER_PID pTable = NULL;
	dwRetValue = GetExtendedUdpTable(NULL, &dwBufferSize,
		FALSE, AF_INET, UDP_TABLE_OWNER_PID, 0);
	if (dwRetValue != ERROR_INSUFFICIENT_BUFFER)
	{
		printf("Failed to snapshot UDP endpoints.\n");
		return;
	}

	//������ͬRebuildTcpTable
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
		int nTemp = ntohs((unsigned short)pTable->table[i].dwLocalPort);
		//_udpPortTable[nTemp] =
		//	pTable->table[i].dwOwningPid;
		_mapUdpPortTableEx[nTemp] = pTable->table[i];
	}
	HeapFree(GetProcessHeap(), 0, pTable);
}