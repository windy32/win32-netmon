#ifndef PROCESS_VIEW_H
#define PROCESS_VIEW_H

#include "SQLite.h"
#include "Packet.h"
#include "ProcessModel.h"

class ProcessView
{
private:
	// It's associated with a ListView control
	static HWND _hList;

private: // Utils Used by ProcessView
	static void ListViewInsert(const ProcessModel::ProcessItem &item);
	static void ListViewUpdate(int index, const ProcessModel::ProcessItem &item);

public:
	static void Init(HWND hList);
	static void Update();
};

#endif