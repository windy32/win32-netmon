#ifndef PROCESS_CACHE_H
#define PROCESS_CACHE_H

class ProcessCache
{
protected:
    // The process cache that makes looking up process list much more faster.
    // The table is initialized to all zero in constructor, which means that
    // no map from pid to name or full path is logged.
    //
    // ProcessCache class has a GetName() / GetFullPath() interface, which is used to replace
    // CreateToolhelp32Snapshot / Process32First / Process32Next / OpenProcess / GetModuleFileNameEx
    // for better performance.
    // 
    // If the map from pid to name and full path is logged in the table, ProcessCache will return quickly.
    // 
    // If not, ProcessCache calls CreateToolhelp32Snapshot / Process32First / Process32Next / 
    // OpenProcess / GetModuleFileNameEx to update the pid table, and then return the result.
    //
    // Return Value
    //
    //     GetName / GetFullPath returns the name / full path for the corresponding pid.
    //     If the name or full path cannot be found, return value is NULL.
    typedef struct tagProcessInfo
    {
        TCHAR name[MAX_PATH];
        TCHAR path[MAX_PATH];
    } ProcessInfo;

    std::map<int, ProcessInfo> _processTable;

    CRITICAL_SECTION _cs;

    void rebuildTable(bool dump = true);

    static ProcessCache *_instance;
    ProcessCache();
    ~ProcessCache();

public:
    static ProcessCache *instance();

    void GetName(int pid, TCHAR *buf, int cchLen);
    void GetFullPath(int pid, TCHAR *buf, int cchLen);
    BOOL IsProcessAlive(int pid, const TCHAR *name, bool rebuild);
};

#endif
