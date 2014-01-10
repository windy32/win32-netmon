#ifndef PROCESS_MODEL_H
#define PROCESS_MODEL_H

#include "SQLite.h"
#include "Packet.h"

class ProcessModel
{
public:
    // The Process class maintains a list of process items
    typedef struct tagProcessItem
    {
        // Static Parameters
        bool active;

        int  puid;
        TCHAR name[MAX_PATH];
        TCHAR fullPath[MAX_PATH];

        bool hidden;

        // Valid Only When Active
        bool dirty;

        int txRate;
        int rxRate;
        int prevTxRate;
        int prevRxRate;

        int pid; // Note: A "Process" item may corresponds to multiple pids
    } ProcessItem;

private:
    static std::vector<ProcessItem> _processes;
    static CRITICAL_SECTION _cs;

private:
    static void InitCallback(SQLiteRow *row);
    static void Lock();
    static void Unlock();

public:    
    // Modify the Model
    static void Init();
    static void OnPacket(PacketInfoEx *pi);
    static void OnTimer();

    static void ResetDirty(int puid);
    static void ShowProcess(int puid);
    static void HideProcess(int puid);

    // Export Model Info
    static void Export(std::vector<ProcessItem> &items);
    static void ExportHiddenState(std::vector<bool> &states);
    static void ExportHiddenProcesses(std::vector<int> &processes);

public:
    static int  GetProcessUid(int index);
    static int  GetProcessCount();
    static int  GetProcessUid(const TCHAR *name);
    static bool GetProcessName(int puid, TCHAR *buf, int len);
    static int  GetProcessIndex(int puid);
    static bool GetProcessRate(int puid, int *txRate, int *rxRate);
    static bool IsProcessActive(int puid);
};

#endif