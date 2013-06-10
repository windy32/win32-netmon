#include "stdafx.h"
#include "ProcessView.h"
#include "Utils.h"

HWND ProcessView::_hList;

// Initialize process list & ListView control
void ProcessView::Init(HWND hList)
{
	// Init ListView Headers
	_hList = hList;

	Utils::ListViewInit(_hList, FALSE, 5, 
		TEXT("UID"), TEXT("Process"), TEXT("Tx Rate"), TEXT("Rx Rate"), TEXT("Full Path"), 
		50, 140, 70, 70, 300);

	// Init Model
	ProcessModel::Init();
}

// ListView operations - insert an item & update an item
void ProcessView::ListViewInsert(const ProcessModel::ProcessItem &item)
{
	// Prepare Columns
	TCHAR szColumn[5][MAX_PATH];

	_stprintf_s(szColumn[0], MAX_PATH, TEXT("%d"), item.puid);
	_stprintf_s(szColumn[1], MAX_PATH, TEXT("%s"), item.name);

	if( item.active )
	{
		_stprintf_s(szColumn[2], MAX_PATH, TEXT("%d"), item.prevTxRate);
		_stprintf_s(szColumn[3], MAX_PATH, TEXT("%d"), item.prevRxRate);
		_tcscpy_s(szColumn[4], MAX_PATH, item.fullPath);
	}
	else
	{
		_tcscpy_s(szColumn[2], MAX_PATH, TEXT("-"));
		_tcscpy_s(szColumn[3], MAX_PATH, TEXT("-"));
		_tcscpy_s(szColumn[4], MAX_PATH, TEXT("-"));
	}

	Utils::ListViewAppend(_hList, 5, 
		szColumn[0], szColumn[1], szColumn[2], szColumn[3], szColumn[4]);
}

void ProcessView::ListViewUpdate(int index, const ProcessModel::ProcessItem &item)
{
	// Prepare Columns
	TCHAR szColumn[5][MAX_PATH];

	if( item.active )
	{
		_stprintf_s(szColumn[2], MAX_PATH, TEXT("%d.%d"), item.prevTxRate / 1024, (item.prevTxRate % 1024 + 51) / 108);
		_stprintf_s(szColumn[3], MAX_PATH, TEXT("%d.%d"), item.prevRxRate / 1024, (item.prevRxRate % 1024 + 51) / 108);
		_tcscpy_s(szColumn[4], MAX_PATH, item.fullPath);
	}
	else
	{
		_tcscpy_s(szColumn[2], MAX_PATH, TEXT("-"));
		_tcscpy_s(szColumn[3], MAX_PATH, TEXT("-"));
		_tcscpy_s(szColumn[4], MAX_PATH, TEXT("-"));
	}

	Utils::ListViewUpdate(_hList, index, 5, 
		FALSE, FALSE, TRUE, TRUE, TRUE,
		szColumn[0], szColumn[1], szColumn[2], szColumn[3], szColumn[4]);
}

void ProcessView::Update(bool redraw)
{
	// Update is called in:
	//   1. ProcessModel::Init
	//   2. ProcessModel::OnTimer
	std::vector<ProcessModel::ProcessItem> processes;
	ProcessModel::Export(processes);

	if (Utils::ListViewGetRowCount(_hList) == 0) // Init
	{
		for (unsigned int i = 0; i < processes.size(); i++)
		{
			ListViewInsert(processes[i]);
		}
	}
	else // OnTimer
	{
		int rows = Utils::ListViewGetRowCount(_hList);
		for (int i = 0; i < rows; i++)
		{
			if (processes[i].dirty)
			{
				ListViewUpdate(i, processes[i]);
			}
		}

		for (unsigned int i = rows; i < processes.size(); i++)
		{
			ListViewInsert(processes[i]);
		}
	}

	if (redraw)
	{
		RedrawWindow(_hList, NULL, NULL, RDW_INVALIDATE);
	}
}
